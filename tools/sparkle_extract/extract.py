import argparse
from pathlib import Path

import numpy as np
from PIL import Image

BACKGROUND_FLOOR = 6.0 / 255.0
CHECKER_SIZE = 16
DARK_BACKGROUND = np.array([16, 16, 20], dtype=np.uint8)


def loadRgb(path):
    with Image.open(path) as image:
        return np.asarray(image.convert("RGB"), dtype=np.float32) / 255.0


def keyFromBlack(rgb, floor):
    peak = rgb.max(axis=2)
    alpha = np.clip((peak - floor) / (1.0 - floor), 0.0, 1.0)
    safePeak = np.maximum(peak, 1e-4)[..., None]
    color = np.clip(rgb / safePeak, 0.0, 1.0)
    return color, alpha


def toRgba8(color, alpha):
    rgba = np.empty(color.shape[:2] + (4,), dtype=np.uint8)
    rgba[..., :3] = np.rint(color * 255.0).astype(np.uint8)
    rgba[..., 3] = np.rint(alpha * 255.0).astype(np.uint8)
    return rgba


def splitCells(image, columns, rows):
    height, width = image.shape[:2]
    cellHeight, cellWidth = height // rows, width // columns
    cells = []
    for row in range(rows):
        for column in range(columns):
            cells.append(
                image[
                    row * cellHeight:(row + 1) * cellHeight,
                    column * cellWidth:(column + 1) * cellWidth,
                ]
            )
    return cells


def checkerboard(height, width, size, light, dark):
    rows = (np.arange(height) // size)[:, None]
    columns = (np.arange(width) // size)[None, :]
    pattern = ((rows + columns) % 2).astype(bool)
    board = np.empty((height, width, 3), dtype=np.uint8)
    board[pattern] = light
    board[~pattern] = dark
    return board


def composite(rgba, background):
    alpha = rgba[..., 3:4].astype(np.float32) / 255.0
    color = rgba[..., :3].astype(np.float32)
    output = color * alpha + background.astype(np.float32) * (1.0 - alpha)
    return np.clip(np.rint(output), 0, 255).astype(np.uint8)


def buildPreview(frames, columns, rows, background):
    height, width = frames[0].shape[:2]
    preview = np.empty((height * rows, width * columns, 3), dtype=np.uint8)
    for index, frame in enumerate(frames):
        row, column = divmod(index, columns)
        region = (
            slice(row * height, (row + 1) * height),
            slice(column * width, (column + 1) * width),
        )
        preview[region] = composite(frame, background[region])
    return preview


def saveGif(frames, path, delay, background):
    images = [
        Image.fromarray(composite(frame, background)).convert("P", palette=Image.Palette.ADAPTIVE)
        for frame in frames
    ]
    images[0].save(path, save_all=True, append_images=images[1:], duration=delay, loop=0, optimize=False)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--source", required=True, type=Path)
    parser.add_argument("--out", required=True, type=Path)
    parser.add_argument("--preview-dir", type=Path)
    parser.add_argument("--columns", type=int, default=4)
    parser.add_argument("--rows", type=int, default=4)
    parser.add_argument("--floor", type=float, default=BACKGROUND_FLOOR)
    parser.add_argument("--delay", type=int, default=70)
    parser.add_argument("--prefix", default="sparkle")
    arguments = parser.parse_args()

    previewDir = arguments.preview_dir or arguments.out
    arguments.out.mkdir(parents=True, exist_ok=True)
    previewDir.mkdir(parents=True, exist_ok=True)

    color, alpha = keyFromBlack(loadRgb(arguments.source), arguments.floor)
    rgba = toRgba8(color, alpha)
    frames = splitCells(rgba, arguments.columns, arguments.rows)

    Image.fromarray(rgba).save(arguments.out / f"{arguments.prefix}_sheet.png")
    for index, frame in enumerate(frames):
        Image.fromarray(frame).save(arguments.out / f"{arguments.prefix}_{index:02d}.png")

    height, width = frames[0].shape[:2]
    checker = checkerboard(height * arguments.rows, width * arguments.columns, CHECKER_SIZE, 200, 250)
    dark = np.full(
        (height * arguments.rows, width * arguments.columns, 3),
        DARK_BACKGROUND,
        dtype=np.uint8,
    )
    Image.fromarray(buildPreview(frames, arguments.columns, arguments.rows, checker)).save(
        previewDir / f"{arguments.prefix}_preview_checker.png"
    )
    Image.fromarray(buildPreview(frames, arguments.columns, arguments.rows, dark)).save(
        previewDir / f"{arguments.prefix}_preview_dark.png"
    )
    saveGif(frames, previewDir / f"{arguments.prefix}_preview.gif", arguments.delay, DARK_BACKGROUND)

    print(f"frames {len(frames)} cell {width}x{height} out {arguments.out}")


if __name__ == "__main__":
    main()
