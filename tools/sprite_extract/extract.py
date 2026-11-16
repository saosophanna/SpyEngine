import argparse
from pathlib import Path

import numpy as np
from PIL import Image

DARK_BACKGROUND = np.array([16, 16, 20], dtype=np.uint8)
CHECKER_SIZE = 16


def loadRgb(path):
    with Image.open(path) as image:
        return np.asarray(image.convert("RGB"), dtype=np.float32) / 255.0


def estimateBackground(rgb, band):
    border = np.concatenate(
        [
            rgb[:band].reshape(-1, 3),
            rgb[-band:].reshape(-1, 3),
            rgb[:, :band].reshape(-1, 3),
            rgb[:, -band:].reshape(-1, 3),
        ]
    )
    return np.median(border, axis=0)


def colorDistance(rgb, background):
    return np.sqrt(((rgb - background) ** 2).sum(axis=2)) / np.sqrt(3.0) * 255.0


def directionalFill(mask, allowed, reverse):
    if reverse:
        mask = mask[:, ::-1]
        allowed = allowed[:, ::-1]
    index = np.arange(mask.shape[1])
    barrier = np.maximum.accumulate(np.where(~allowed, index, -1), axis=1)
    seeded = np.maximum.accumulate(np.where(mask, index, -1), axis=1)
    filled = mask | (allowed & (seeded > barrier))
    if reverse:
        filled = filled[:, ::-1]
    return filled


def flood(seed, allowed):
    mask = seed & allowed
    while True:
        count = int(mask.sum())
        mask = directionalFill(mask, allowed, False)
        mask = directionalFill(mask, allowed, True)
        mask = directionalFill(mask.T, allowed.T, False).T
        mask = directionalFill(mask.T, allowed.T, True).T
        if int(mask.sum()) == count:
            return mask


def borderReachable(allowed):
    seed = np.zeros(allowed.shape, dtype=bool)
    seed[0, :] = seed[-1, :] = seed[:, 0] = seed[:, -1] = True
    return flood(seed, allowed)


def labelRegions(mask):
    labels = np.zeros(mask.shape, dtype=np.int32)
    parent = [0]

    def find(value):
        while parent[value] != value:
            parent[value] = parent[parent[value]]
            value = parent[value]
        return value

    def union(first, second):
        rootFirst, rootSecond = find(first), find(second)
        if rootFirst != rootSecond:
            parent[max(rootFirst, rootSecond)] = min(rootFirst, rootSecond)

    previousRuns = []
    count = 0
    for row in range(mask.shape[0]):
        padded = np.concatenate(([False], mask[row], [False]))
        edges = np.flatnonzero(padded[1:] != padded[:-1])
        runs = []
        cursor = 0
        for start, end in zip(edges[0::2], edges[1::2]):
            count += 1
            parent.append(count)
            while cursor < len(previousRuns) and previousRuns[cursor][1] <= start:
                cursor += 1
            probe = cursor
            while probe < len(previousRuns) and previousRuns[probe][0] < end:
                union(count, previousRuns[probe][2])
                probe += 1
            labels[row, start:end] = count
            runs.append((int(start), int(end), count))
        previousRuns = runs

    if count == 0:
        return labels, 0

    roots = np.array([find(index) for index in range(count + 1)], dtype=np.int32)
    roots[0] = 0
    remap = np.zeros(count + 1, dtype=np.int32)
    uniqueRoots = np.unique(roots[1:])
    for index, root in enumerate(uniqueRoots, start=1):
        remap[root] = index
    return remap[roots[labels]], int(len(uniqueRoots))


def smoothstep(value):
    return value * value * (3.0 - 2.0 * value)


def segment(rgb, background, lowThreshold, highThreshold, gapTolerance):
    dist = colorDistance(rgb, background)
    span = max(highThreshold - lowThreshold, 1e-3)
    alpha = smoothstep(np.clip((dist - lowThreshold) / span, 0.0, 1.0))
    core = dist > highThreshold
    enclosed = ~borderReachable(~core) & ~core
    if enclosed.any():
        labels, count = labelRegions(enclosed)
        means = np.bincount(labels.ravel(), weights=dist.ravel(), minlength=count + 1)
        areas = np.bincount(labels.ravel(), minlength=count + 1)
        backgroundRegions = (means / np.maximum(areas, 1)) < gapTolerance
        backgroundRegions[0] = True
        highlight = (labels > 0) & ~backgroundRegions[labels]
        alpha = np.where(highlight, 1.0, alpha)
    return alpha


def decontaminate(rgb, background, alpha):
    safe = np.maximum(alpha, 1e-3)[..., None]
    color = (rgb - background * (1.0 - alpha[..., None])) / safe
    return np.clip(color, 0.0, 1.0)


def toRgba8(color, alpha):
    rgba = np.empty(color.shape[:2] + (4,), dtype=np.uint8)
    rgba[..., :3] = np.rint(color * 255.0).astype(np.uint8)
    rgba[..., 3] = np.rint(alpha * 255.0).astype(np.uint8)
    return rgba


def bounds(mask):
    rows, columns = np.nonzero(mask)
    return int(columns.min()), int(rows.min()), int(columns.max()), int(rows.max())


def groupCells(labels, count, columns, rows, minArea):
    height, width = labels.shape
    groups = {}
    dropped = 0
    areas = np.bincount(labels.ravel(), minlength=count + 1)
    centersY = np.bincount(labels.ravel(), weights=np.repeat(np.arange(height), width), minlength=count + 1)
    centersX = np.bincount(labels.ravel(), weights=np.tile(np.arange(width), height), minlength=count + 1)
    for label in range(1, count + 1):
        if areas[label] < minArea:
            dropped += 1
            continue
        column = min(int(centersX[label] / areas[label] / (width / columns)), columns - 1)
        row = min(int(centersY[label] / areas[label] / (height / rows)), rows - 1)
        key = row * columns + column
        groups.setdefault(key, []).append(label)
    masks = {}
    for key, members in groups.items():
        masks[key] = np.isin(labels, members)
    return masks, dropped


def shiftMask(array, axis, offset):
    output = np.zeros_like(array)
    if axis == 0:
        if offset > 0:
            output[offset:, :] = array[:-offset, :]
        else:
            output[:offset, :] = array[-offset:, :]
    else:
        if offset > 0:
            output[:, offset:] = array[:, :-offset]
        else:
            output[:, :offset] = array[:, -offset:]
    return output


def assignOwners(groups, candidate, radius):
    owner = np.zeros(candidate.shape, dtype=np.int32)
    for key, mask in groups.items():
        owner[mask] = key + 1
    for _ in range(radius):
        previous = owner.copy()
        for axis, offset in ((0, 1), (0, -1), (1, 1), (1, -1)):
            neighbour = shiftMask(owner, axis, offset)
            take = (owner == 0) & candidate & (neighbour > 0)
            owner[take] = neighbour[take]
        if np.array_equal(owner, previous):
            break
    return owner


def placeOnCanvas(color, alpha, region, canvas, padding):
    x0, y0, x1, y1 = bounds(region)
    cropAlpha = np.where(region[y0:y1 + 1, x0:x1 + 1], alpha[y0:y1 + 1, x0:x1 + 1], 0.0)
    cropColor = color[y0:y1 + 1, x0:x1 + 1]
    limit = canvas - 2 * padding
    scale = min(1.0, limit / max(cropAlpha.shape))
    if scale < 1.0:
        size = (
            max(1, int(round(cropAlpha.shape[1] * scale))),
            max(1, int(round(cropAlpha.shape[0] * scale))),
        )
        cropAlpha = np.asarray(
            Image.fromarray(cropAlpha.astype(np.float32), mode="F").resize(size, Image.LANCZOS)
        )
        cropColor = np.stack(
            [
                np.asarray(
                    Image.fromarray(cropColor[..., channel].astype(np.float32), mode="F").resize(
                        size, Image.LANCZOS
                    )
                )
                for channel in range(3)
            ],
            axis=2,
        )
        cropAlpha = np.clip(cropAlpha, 0.0, 1.0)
        cropColor = np.clip(cropColor, 0.0, 1.0)
    canvasColor = np.zeros((canvas, canvas, 3), dtype=np.float32)
    canvasAlpha = np.zeros((canvas, canvas), dtype=np.float32)
    offsetY = (canvas - cropAlpha.shape[0]) // 2
    offsetX = (canvas - cropAlpha.shape[1]) // 2
    canvasAlpha[offsetY:offsetY + cropAlpha.shape[0], offsetX:offsetX + cropAlpha.shape[1]] = cropAlpha
    canvasColor[offsetY:offsetY + cropAlpha.shape[0], offsetX:offsetX + cropAlpha.shape[1]] = cropColor
    return canvasColor, canvasAlpha


def composite(rgba, background):
    alpha = rgba[..., 3:4].astype(np.float32) / 255.0
    color = rgba[..., :3].astype(np.float32)
    output = color * alpha + background.astype(np.float32) * (1.0 - alpha)
    return np.clip(np.rint(output), 0, 255).astype(np.uint8)


def checkerboard(height, width, size, light, dark):
    rows = (np.arange(height) // size)[:, None]
    columns = (np.arange(width) // size)[None, :]
    pattern = ((rows + columns) % 2).astype(bool)
    board = np.empty((height, width, 3), dtype=np.uint8)
    board[pattern] = light
    board[~pattern] = dark
    return board


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--source", required=True, type=Path)
    parser.add_argument("--out", required=True, type=Path)
    parser.add_argument("--preview-dir", type=Path)
    parser.add_argument("--names", default="")
    parser.add_argument("--columns", type=int, default=5)
    parser.add_argument("--rows", type=int, default=2)
    parser.add_argument("--low", type=float, default=3.0)
    parser.add_argument("--high", type=float, default=20.0)
    parser.add_argument("--gap-tolerance", type=float, default=6.0)
    parser.add_argument("--min-area", type=int, default=24)
    parser.add_argument("--spread", type=int, default=6)
    parser.add_argument("--canvas", type=int, default=0)
    parser.add_argument("--padding", type=float, default=0.07)
    parser.add_argument("--prefix", default="sprite")
    arguments = parser.parse_args()

    previewDir = arguments.preview_dir or arguments.out
    arguments.out.mkdir(parents=True, exist_ok=True)
    previewDir.mkdir(parents=True, exist_ok=True)

    rgb = loadRgb(arguments.source)
    background = estimateBackground(rgb, 3)
    alpha = segment(rgb, background, arguments.low, arguments.high, arguments.gap_tolerance)
    color = decontaminate(rgb, background, alpha)

    candidate = alpha > 0.0
    labels, count = labelRegions(alpha > 0.5)
    groups, dropped = groupCells(labels, count, arguments.columns, arguments.rows, arguments.min_area)
    owner = assignOwners(groups, candidate, arguments.spread)
    regions = {key: (owner == key + 1) & candidate for key in groups}
    print(
        f"background {(background * 255).round(1).tolist()} components {count} "
        f"dropped {dropped} groups {len(groups)}"
    )

    names = [name.strip() for name in arguments.names.split(",") if name.strip()]
    maxExtent = 0
    for key, region in regions.items():
        x0, y0, x1, y1 = bounds(region)
        maxExtent = max(maxExtent, x1 - x0 + 1, y1 - y0 + 1)
    padding = max(2, int(round(maxExtent * arguments.padding)))
    canvasSize = arguments.canvas or int(np.ceil((maxExtent + 2 * padding) / 8.0) * 8)

    placed = {}
    for key, region in regions.items():
        placed[key] = toRgba8(*placeOnCanvas(color, alpha, region, canvasSize, padding))

    ordered = []
    for key in range(arguments.rows * arguments.columns):
        if key not in placed:
            continue
        name = names[key] if key < len(names) else f"{arguments.prefix}_{key:02d}"
        Image.fromarray(placed[key]).save(arguments.out / f"{name}.png")
        ordered.append((key, placed[key]))
        print(f"{key:2d} {name:16s} {canvasSize}x{canvasSize}")

    sheet = np.zeros((canvasSize * arguments.rows, canvasSize * arguments.columns, 4), dtype=np.uint8)
    for key, image in ordered:
        row, column = divmod(key, arguments.columns)
        sheet[
            row * canvasSize:(row + 1) * canvasSize,
            column * canvasSize:(column + 1) * canvasSize,
        ] = image
    Image.fromarray(sheet).save(arguments.out / f"{arguments.prefix}_sheet.png")

    dark = np.full(sheet.shape[:2] + (3,), DARK_BACKGROUND, dtype=np.uint8)
    checker = checkerboard(sheet.shape[0], sheet.shape[1], CHECKER_SIZE, 200, 250)
    Image.fromarray(composite(sheet, checker)).save(previewDir / f"{arguments.prefix}_preview_checker.png")
    Image.fromarray(composite(sheet, dark)).save(previewDir / f"{arguments.prefix}_preview_dark.png")
    print(f"canvas {canvasSize} sheet {sheet.shape[1]}x{sheet.shape[0]} out {arguments.out}")


if __name__ == "__main__":
    main()
