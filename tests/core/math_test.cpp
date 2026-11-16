#include "core/math/math.hpp"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <vector>

namespace {

int g_failures = 0;

void check(bool condition, const char* expression, int line)
{
    if (!condition)
    {
        std::fprintf(stderr, "FAILED: %s (line %d)\n", expression, line);
        ++g_failures;
    }
}

bool approx(float first, float second, float tolerance = 1.0e-4f)
{
    return spy::math::nearlyEqual(first, second, tolerance);
}

bool approx(const spy::math::Vec2& first, const spy::math::Vec2& second,
            float tolerance = 1.0e-4f)
{
    return spy::math::nearlyEqual(first, second, tolerance);
}

bool approx(const spy::math::Vec3& first, const spy::math::Vec3& second,
            float tolerance = 1.0e-4f)
{
    return spy::math::nearlyEqual(first, second, tolerance);
}

bool approx(const spy::math::Vec4& first, const spy::math::Vec4& second,
            float tolerance = 1.0e-4f)
{
    return spy::math::nearlyEqual(first, second, tolerance);
}

bool approx(const spy::math::Mat3& first, const spy::math::Mat3& second,
            float tolerance = 1.0e-4f)
{
    return spy::math::nearlyEqual(first, second, tolerance);
}

bool approx(const spy::math::Mat4& first, const spy::math::Mat4& second,
            float tolerance = 1.0e-4f)
{
    return spy::math::nearlyEqual(first, second, tolerance);
}

bool approx(const spy::math::Quaternion& first, const spy::math::Quaternion& second,
            float tolerance = 1.0e-4f)
{
    return spy::math::nearlyEqual(first, second, tolerance);
}

}

#define SPY_CHECK(expr) check((expr), #expr, __LINE__)

int main()
{
    using namespace spy::math;

    {
        SPY_CHECK(approx(constants::kPi, 3.14159265f));
        SPY_CHECK(approx(constants::kTwoPi, 6.28318530f));
        SPY_CHECK(approx(constants::kHalfPi, 1.57079632f));

        SPY_CHECK(approx(clamp(5.0f, 0.0f, 1.0f), 1.0f));
        SPY_CHECK(approx(clamp(-2.0f, 0.0f, 1.0f), 0.0f));
        SPY_CHECK(approx(clamp(0.25f, 0.0f, 1.0f), 0.25f));
        SPY_CHECK(approx(minValue(3.0f, 7.0f), 3.0f));
        SPY_CHECK(approx(maxValue(3.0f, 7.0f), 7.0f));
        SPY_CHECK(approx(absValue(-3.5f), 3.5f));
        SPY_CHECK(approx(sign(-3.0f), -1.0f));
        SPY_CHECK(approx(sign(0.0f), 0.0f));
        SPY_CHECK(approx(sign(3.0f), 1.0f));
        SPY_CHECK(approx(saturate(1.5f), 1.0f));
        SPY_CHECK(approx(saturate(-0.5f), 0.0f));
        SPY_CHECK(approx(square(4.0f), 16.0f));

        SPY_CHECK(approx(wrap(-0.25f, 0.0f, 1.0f), 0.75f));
        SPY_CHECK(approx(wrap(1.25f, 0.0f, 1.0f), 0.25f));
        SPY_CHECK(approx(wrap(2.5f, 0.0f, 1.0f), 0.5f));

        SPY_CHECK(nearlyEqual(1.0f, 1.0f));
        SPY_CHECK(nearlyEqual(1.0f, 1.0f + 1.0e-7f));
        SPY_CHECK(!nearlyEqual(1.0f, 2.0f));
        SPY_CHECK(nearlyEqual(1.0e8f, 1.0e8f + 10.0f));

        SPY_CHECK(approx(degreesToRadians(180.0f), constants::kPi));
        SPY_CHECK(approx(radiansToDegrees(constants::kPi), 180.0f));

        SPY_CHECK(isPowerOfTwo(1U));
        SPY_CHECK(isPowerOfTwo(64U));
        SPY_CHECK(!isPowerOfTwo(0U));
        SPY_CHECK(!isPowerOfTwo(48U));
        SPY_CHECK(nextPowerOfTwo(0U) == 1U);
        SPY_CHECK(nextPowerOfTwo(1U) == 1U);
        SPY_CHECK(nextPowerOfTwo(65U) == 128U);
    }

    {
        const Angle quarterTurn = Angle::fromDegrees(90.0f);
        SPY_CHECK(approx(quarterTurn.radians, constants::kHalfPi));
        SPY_CHECK(approx(quarterTurn.degrees(), 90.0f));

        const Angle normalized = Angle::fromDegrees(370.0f).normalized();
        SPY_CHECK(approx(normalized.degrees(), 10.0f));

        SPY_CHECK(approx(angleDifference(0.0f, constants::kHalfPi), constants::kHalfPi));

        const Angle sum = Angle::fromRadians(1.0f) + Angle::fromRadians(2.0f);
        SPY_CHECK(approx(sum.radians, 3.0f));
    }

    {
        Random first(1234U);
        Random second(1234U);

        for (int index = 0; index < 8; ++index)
        {
            SPY_CHECK(first.nextUint64() == second.nextUint64());
        }

        Random seeded(5678U);

        for (int index = 0; index < 100; ++index)
        {
            const float value = seeded.nextFloat();
            SPY_CHECK(value >= 0.0f && value < 1.0f);

            const float ranged = seeded.rangeFloat(2.0f, 3.0f);
            SPY_CHECK(ranged >= 2.0f && ranged <= 3.0f);

            const std::uint32_t integer = seeded.rangeUint(1U, 6U);
            SPY_CHECK(integer >= 1U && integer <= 6U);
        }

        SPY_CHECK(!seeded.chance(0.0f));
        SPY_CHECK(seeded.chance(1.0f));
    }

    {
        const Vec2 first{1.0f, 2.0f};
        const Vec2 second{3.0f, 4.0f};

        SPY_CHECK(approx(first + second, Vec2{4.0f, 6.0f}));
        SPY_CHECK(approx(first - second, Vec2{-2.0f, -2.0f}));
        SPY_CHECK(approx(-first, Vec2{-1.0f, -2.0f}));
        SPY_CHECK(approx(first * 2.0f, Vec2{2.0f, 4.0f}));
        SPY_CHECK(approx(2.0f * first, Vec2{2.0f, 4.0f}));
        SPY_CHECK(approx(first / 2.0f, Vec2{0.5f, 1.0f}));

        Vec2 accumulated = first;
        accumulated += second;
        SPY_CHECK(approx(accumulated, Vec2{4.0f, 6.0f}));
        accumulated -= second;
        SPY_CHECK(approx(accumulated, first));
        accumulated *= 3.0f;
        SPY_CHECK(approx(accumulated, Vec2{3.0f, 6.0f}));
        accumulated /= 3.0f;
        SPY_CHECK(approx(accumulated, first));

        SPY_CHECK(approx(first.dot(second), 11.0f));
        SPY_CHECK(approx(first.cross(second), -2.0f));

        const Vec2 diagonal{3.0f, 4.0f};
        SPY_CHECK(approx(diagonal.lengthSquared(), 25.0f));
        SPY_CHECK(approx(diagonal.length(), 5.0f));
        SPY_CHECK(approx(diagonal.normalized().length(), 1.0f));
        SPY_CHECK(approx(diagonal.distance(Vec2::zero()), 5.0f));
        SPY_CHECK(approx(diagonal.distanceSquared(Vec2::zero()), 25.0f));
        SPY_CHECK(approx(Vec2::distance(Vec2::zero(), Vec2{3.0f, 4.0f}), 5.0f));

        Vec2 zero = Vec2::zero();
        SPY_CHECK(!zero.normalize());
        SPY_CHECK(zero == Vec2::zero());
        SPY_CHECK(zero.normalized() == Vec2::zero());

        SPY_CHECK(approx(Vec2::unitX().perpendicular(), Vec2::unitY()));
        SPY_CHECK(approx(Vec2::unitY().angle(), constants::kHalfPi));
        SPY_CHECK(approx(Vec2::unitX().rotated(constants::kHalfPi), Vec2::unitY()));
        SPY_CHECK(approx(rotate(Vec2::unitX(), constants::kHalfPi), Vec2::unitY()));

        SPY_CHECK(approx(project(Vec2{3.0f, 4.0f}, Vec2::unitX()), Vec2{3.0f, 0.0f}));
        SPY_CHECK(approx(reflect(Vec2{1.0f, -1.0f}, Vec2::unitY()), Vec2{1.0f, 1.0f}));
    }

    {
        const Vec3 first{1.0f, 2.0f, 3.0f};
        const Vec3 second{4.0f, 5.0f, 6.0f};

        SPY_CHECK(approx(first + second, Vec3{5.0f, 7.0f, 9.0f}));
        SPY_CHECK(approx(first.dot(second), 32.0f));
        SPY_CHECK(approx(Vec3::unitX().cross(Vec3::unitY()), Vec3::unitZ()));
        SPY_CHECK(approx(Vec3::unitY().cross(Vec3::unitX()), -Vec3::unitZ()));
        SPY_CHECK(approx(Vec3{3.0f, 0.0f, 4.0f}.length(), 5.0f));
        SPY_CHECK(approx(Vec3::distance(Vec3::zero(), Vec3{3.0f, 0.0f, 4.0f}), 5.0f));
        SPY_CHECK(approx(Vec3{3.0f, 0.0f, 4.0f}.normalized().length(), 1.0f));
        SPY_CHECK(approx(Vec3{Vec2{1.0f, 2.0f}, 3.0f}.xy(), Vec2{1.0f, 2.0f}));
        SPY_CHECK(approx(reflect(Vec3{1.0f, -1.0f, 0.0f}, Vec3::unitY()),
                         Vec3{1.0f, 1.0f, 0.0f}));
    }

    {
        const Vec4 first{1.0f, 2.0f, 3.0f, 4.0f};
        const Vec4 second{4.0f, 3.0f, 2.0f, 1.0f};

        SPY_CHECK(approx(first + second, Vec4{5.0f, 5.0f, 5.0f, 5.0f}));
        SPY_CHECK(approx(first.dot(second), 20.0f));
        SPY_CHECK(approx(first.xyz(), Vec3{1.0f, 2.0f, 3.0f}));
        SPY_CHECK(approx(Vec4{3.0f, 0.0f, 0.0f, 4.0f}.length(), 5.0f));
        SPY_CHECK(approx(Vec4::distance(Vec4::zero(), Vec4{3.0f, 0.0f, 0.0f, 4.0f}), 5.0f));
        SPY_CHECK(approx(first * 2.0f, Vec4{2.0f, 4.0f, 6.0f, 8.0f}));
    }

    {
        const Mat3 translation = Mat3::translation(Vec2{2.0f, 3.0f});
        const Mat3 rotation = Mat3::rotation(constants::kHalfPi);
        const Mat3 scaling = Mat3::scaling(Vec2{2.0f, 2.0f});

        SPY_CHECK(approx(Mat3::identity() * translation, translation));
        SPY_CHECK(approx(translation.transformPoint(Vec2{1.0f, 1.0f}), Vec2{3.0f, 4.0f}));
        SPY_CHECK(approx(translation.transformVector(Vec2{1.0f, 1.0f}), Vec2{1.0f, 1.0f}));
        SPY_CHECK(approx(rotation.transformPoint(Vec2::unitX()), Vec2::unitY()));
        SPY_CHECK(approx(scaling.transformPoint(Vec2{1.0f, 1.0f}), Vec2{2.0f, 2.0f}));

        const Mat3 composed = translation * rotation * scaling;
        SPY_CHECK(approx(composed.transformPoint(Vec2{1.0f, 0.0f}), Vec2{2.0f, 5.0f}));
        SPY_CHECK(approx((translation * rotation) * scaling, translation * (rotation * scaling)));

        SPY_CHECK(approx(rotation.transposed(), Mat3::rotation(-constants::kHalfPi)));
        SPY_CHECK(approx(scaling.determinant(), 4.0f));

        Mat3 inverse;
        SPY_CHECK(composed.inverse(inverse));
        SPY_CHECK(approx(inverse.transformPoint(Vec2{2.0f, 5.0f}), Vec2{1.0f, 0.0f}));

        Mat3 singularInverse;
        SPY_CHECK(!Mat3::scaling(Vec2{0.0f, 1.0f}).inverse(singularInverse));
    }

    {
        const Mat4 translation = Mat4::translation(Vec3{1.0f, 2.0f, 3.0f});
        const Mat4 rotation = Mat4::rotationZ(constants::kHalfPi);
        const Mat4 scaling = Mat4::scaling(Vec3{2.0f, 2.0f, 2.0f});

        SPY_CHECK(approx(Mat4::identity() * translation, translation));
        SPY_CHECK(approx(translation.transformPoint(Vec3{1.0f, 1.0f, 1.0f}),
                         Vec3{2.0f, 3.0f, 4.0f}));
        SPY_CHECK(approx(translation.transformVector(Vec3{1.0f, 1.0f, 1.0f}),
                         Vec3{1.0f, 1.0f, 1.0f}));
        SPY_CHECK(approx(rotation.transformPoint(Vec3::unitX()), Vec3::unitY()));
        SPY_CHECK(approx(Mat4::rotation(constants::kHalfPi, Vec3::unitZ()),
                         Mat4::rotationZ(constants::kHalfPi)));

        const Mat4 composed = translation * rotation * scaling;
        SPY_CHECK(approx(composed.transformPoint(Vec3{1.0f, 0.0f, 0.0f}),
                         Vec3{1.0f, 4.0f, 3.0f}));

        Mat4 inverse;
        SPY_CHECK(composed.inverse(inverse));
        SPY_CHECK(approx(inverse.transformPoint(Vec3{1.0f, 4.0f, 3.0f}),
                         Vec3{1.0f, 0.0f, 0.0f}));

        Mat4 singularInverse;
        SPY_CHECK(!Mat4::scaling(Vec3{0.0f, 1.0f, 1.0f}).inverse(singularInverse));

        const Mat4 view = Mat4::lookAt(Vec3{0.0f, 0.0f, 5.0f}, Vec3::zero(), Vec3::unitY());
        SPY_CHECK(approx(view.transformPoint(Vec3{0.0f, 0.0f, 5.0f}), Vec3::zero()));
        SPY_CHECK(approx(view.transformPoint(Vec3::zero()), Vec3{0.0f, 0.0f, -5.0f}));

        const Mat4 projection = Mat4::perspective(constants::kHalfPi, 1.0f, 0.1f, 100.0f);
        const Vec4 nearPoint = projection.transform(Vec4{0.0f, 0.0f, -0.1f, 1.0f});
        const Vec4 farPoint = projection.transform(Vec4{0.0f, 0.0f, -100.0f, 1.0f});
        SPY_CHECK(approx(nearPoint.z / nearPoint.w, 0.0f));
        SPY_CHECK(approx(farPoint.z / farPoint.w, 1.0f));

        const Mat4 orthographic =
            Mat4::orthographic(-2.0f, 2.0f, -1.0f, 1.0f, 0.1f, 100.0f);
        const Vec4 corner = orthographic.transform(Vec4{-2.0f, -1.0f, -0.1f, 1.0f});
        const Vec4 farCorner = orthographic.transform(Vec4{2.0f, 1.0f, -100.0f, 1.0f});
        SPY_CHECK(approx(corner.x, -1.0f));
        SPY_CHECK(approx(corner.y, -1.0f));
        SPY_CHECK(approx(corner.z, 0.0f));
        SPY_CHECK(approx(farCorner.z, 1.0f));
    }

    {
        const Quaternion identity = Quaternion::identity();
        SPY_CHECK(approx(identity.rotate(Vec3{1.0f, 2.0f, 3.0f}), Vec3{1.0f, 2.0f, 3.0f}));

        const Quaternion quarterTurn = Quaternion::fromAxisAngle(Vec3::unitZ(), constants::kHalfPi);
        SPY_CHECK(approx(quarterTurn.length(), 1.0f));
        SPY_CHECK(approx(quarterTurn.rotate(Vec3::unitX()), Vec3::unitY()));
        SPY_CHECK(approx(quarterTurn.toMat3(), Mat3::rotation(constants::kHalfPi)));
        SPY_CHECK(approx(quarterTurn.toMat4(), Mat4::rotationZ(constants::kHalfPi)));
        SPY_CHECK(approx(quarterTurn * quarterTurn.inverse(), Quaternion::identity()));

        const Quaternion quarterTurnX =
            Quaternion::fromAxisAngle(Vec3::unitX(), constants::kHalfPi);
        const Vec3 vector{0.0f, 1.0f, 0.0f};
        SPY_CHECK(approx((quarterTurn * quarterTurnX).rotate(vector),
                         quarterTurn.rotate(quarterTurnX.rotate(vector))));

        const Quaternion halfway = slerp(Quaternion::identity(), quarterTurn, 0.5f);
        const float rootHalf = std::sqrt(0.5f);
        SPY_CHECK(approx(halfway.rotate(Vec3::unitX()), Vec3{rootHalf, rootHalf, 0.0f}));

        SPY_CHECK(Quaternion::fromAxisAngle(Vec3::zero(), 1.0f) == Quaternion::identity());
    }

    {
        Transform2D transform;
        transform.position = Vec2{10.0f, 0.0f};
        transform.rotation = constants::kHalfPi;
        transform.scale = Vec2{2.0f, 2.0f};

        SPY_CHECK(approx(transform.transformPoint(Vec2{1.0f, 0.0f}), Vec2{10.0f, 2.0f}));
        SPY_CHECK(approx(transform.transformVector(Vec2{1.0f, 0.0f}), Vec2{0.0f, 2.0f}));
        SPY_CHECK(approx(transform.matrix().transformPoint(Vec2{1.0f, 0.0f}),
                         Vec2{10.0f, 2.0f}));

        Vec2 local;
        SPY_CHECK(transform.inverseTransformPoint(Vec2{10.0f, 2.0f}, local));
        SPY_CHECK(approx(local, Vec2{1.0f, 0.0f}));

        transform.scale = Vec2::zero();
        SPY_CHECK(!transform.inverseTransformPoint(Vec2::zero(), local));
    }

    {
        Transform3D transform;
        transform.position = Vec3{0.0f, 0.0f, 5.0f};
        transform.rotation = Quaternion::fromAxisAngle(Vec3::unitZ(), constants::kHalfPi);
        transform.scale = Vec3{2.0f, 2.0f, 2.0f};

        SPY_CHECK(approx(transform.transformPoint(Vec3::unitX()), Vec3{0.0f, 2.0f, 5.0f}));
        SPY_CHECK(approx(transform.transformVector(Vec3::unitX()), Vec3{0.0f, 2.0f, 0.0f}));
        SPY_CHECK(approx(transform.matrix().transformPoint(Vec3::unitX()),
                         Vec3{0.0f, 2.0f, 5.0f}));

        Vec3 local;
        SPY_CHECK(transform.inverseTransformPoint(Vec3{0.0f, 2.0f, 5.0f}, local));
        SPY_CHECK(approx(local, Vec3::unitX()));

        transform.scale = Vec3::zero();
        SPY_CHECK(!transform.inverseTransformPoint(Vec3::zero(), local));
    }

    {
        const Rect rect{Vec2{1.0f, 2.0f}, Vec2{4.0f, 3.0f}};
        SPY_CHECK(approx(rect.center(), Vec2{3.0f, 3.5f}));
        SPY_CHECK(approx(rect.min(), Vec2{1.0f, 2.0f}));
        SPY_CHECK(approx(rect.max(), Vec2{5.0f, 5.0f}));
        SPY_CHECK(approx(rect.width(), 4.0f));
        SPY_CHECK(approx(rect.height(), 3.0f));
        SPY_CHECK(rect.contains(Vec2{1.0f, 2.0f}));
        SPY_CHECK(rect.contains(Vec2{3.0f, 3.0f}));
        SPY_CHECK(!rect.contains(Vec2{6.0f, 3.0f}));
        SPY_CHECK(approx(closestPointOnRect(Vec2{3.0f, 3.0f}, rect), Vec2{3.0f, 3.0f}));
        SPY_CHECK(approx(closestPointOnRect(Vec2{6.0f, 6.0f}, rect), Vec2{5.0f, 5.0f}));
        SPY_CHECK(approx(closestPointOnRect(Vec2{0.0f, 3.0f}, rect), Vec2{1.0f, 3.0f}));

        const Rect touching{Vec2{5.0f, 2.0f}, Vec2{2.0f, 2.0f}};
        const Rect separated{Vec2{5.5f, 2.0f}, Vec2{2.0f, 2.0f}};
        SPY_CHECK(rect.overlaps(touching));
        SPY_CHECK(!rect.overlaps(separated));

        const Rect flipped{Vec2{5.0f, 5.0f}, Vec2{-4.0f, -3.0f}};
        SPY_CHECK(approx(flipped.normalized().min(), Vec2{1.0f, 2.0f}));
        SPY_CHECK(approx(flipped.normalized().size, Vec2{4.0f, 3.0f}));
        SPY_CHECK(rect.contains(Vec2{5.0f, 5.0f}));

        const Circle circle{Vec2{0.0f, 0.0f}, 1.0f};
        SPY_CHECK(circle.contains(Vec2{1.0f, 0.0f}));
        SPY_CHECK(circle.contains(Vec2::zero()));
        SPY_CHECK(!circle.contains(Vec2{2.0f, 0.0f}));
        SPY_CHECK(approx(circle.area(), constants::kPi));

        const Aabb firstBox{Vec3{-1.0f, -1.0f, -1.0f}, Vec3{1.0f, 1.0f, 1.0f}};
        const Aabb secondBox{Vec3{1.0f, 0.0f, 0.0f}, Vec3{3.0f, 2.0f, 2.0f}};
        const Aabb thirdBox{Vec3{2.0f, 0.0f, 0.0f}, Vec3{4.0f, 2.0f, 2.0f}};
        SPY_CHECK(firstBox.overlaps(secondBox));
        SPY_CHECK(!firstBox.overlaps(thirdBox));
        SPY_CHECK(firstBox.contains(Vec3::zero()));
        SPY_CHECK(approx(firstBox.volume(), 8.0f));
        SPY_CHECK(approx(closestPointOnAabb(Vec3{2.0f, 2.0f, 2.0f}, firstBox),
                         Vec3{1.0f, 1.0f, 1.0f}));
        SPY_CHECK(approx(closestPointOnAabb(Vec3::zero(), firstBox), Vec3::zero()));
        SPY_CHECK(approx(closestPointOnAabb(Vec3{-2.0f, 0.0f, 0.0f}, firstBox),
                         Vec3{-1.0f, 0.0f, 0.0f}));
        SPY_CHECK(approx(firstBox.merged(thirdBox).min, Vec3{-1.0f, -1.0f, -1.0f}));
        SPY_CHECK(approx(firstBox.merged(thirdBox).max, Vec3{4.0f, 2.0f, 2.0f}));

        const Ray ray{Vec3{0.0f, 1.0f, 0.0f}, Vec3{0.0f, 0.0f, -1.0f}};
        SPY_CHECK(approx(ray.pointAt(2.0f), Vec3{0.0f, 1.0f, -2.0f}));

        Bounds bounds;
        SPY_CHECK(!bounds.valid());
        bounds.expand(Vec3{1.0f, 2.0f, 3.0f});
        bounds.expand(Vec3{-1.0f, 0.0f, 5.0f});
        SPY_CHECK(bounds.valid());
        SPY_CHECK(approx(bounds.min, Vec3{-1.0f, 0.0f, 3.0f}));
        SPY_CHECK(approx(bounds.max, Vec3{1.0f, 2.0f, 5.0f}));
        SPY_CHECK(approx(bounds.center(), Vec3{0.0f, 1.0f, 4.0f}));
        SPY_CHECK(bounds.contains(Vec3{0.0f, 1.0f, 4.0f}));
        SPY_CHECK(!bounds.contains(Vec3{0.0f, 1.0f, 6.0f}));

        Polygon square;
        square.vertices = {Vec2{0.0f, 0.0f}, Vec2{1.0f, 0.0f}, Vec2{1.0f, 1.0f},
                           Vec2{0.0f, 1.0f}};
        SPY_CHECK(approx(square.signedArea(), 1.0f));
        SPY_CHECK(approx(square.area(), 1.0f));
        SPY_CHECK(approx(square.perimeter(), 4.0f));
        SPY_CHECK(approx(square.centroid(), Vec2{0.5f, 0.5f}));
        SPY_CHECK(square.contains(Vec2{0.5f, 0.5f}));
        SPY_CHECK(square.contains(Vec2{0.0f, 0.5f}));
        SPY_CHECK(!square.contains(Vec2{1.5f, 0.5f}));

        const Rect squareBounds = square.bounds();
        SPY_CHECK(approx(squareBounds.position, Vec2::zero()));
        SPY_CHECK(approx(squareBounds.size, Vec2::one()));
        SPY_CHECK(approx(Polygon{}.bounds().size, Vec2::zero()));

        SPY_CHECK(approx(closestPointOnSegment(Vec2{5.0f, 5.0f}, Vec2{0.0f, 0.0f},
                                               Vec2{10.0f, 0.0f}),
                         Vec2{5.0f, 0.0f}));
        SPY_CHECK(approx(closestPointOnSegment(Vec2{15.0f, 3.0f}, Vec2{0.0f, 0.0f},
                                               Vec2{10.0f, 0.0f}),
                         Vec2{10.0f, 0.0f}));
        SPY_CHECK(approx(closestPointOnSegment(Vec2{-5.0f, 3.0f}, Vec2{0.0f, 0.0f},
                                               Vec2{10.0f, 0.0f}),
                         Vec2{0.0f, 0.0f}));
        SPY_CHECK(approx(closestPointOnSegment(Vec2{1.0f, 1.0f}, Vec2{2.0f, 2.0f},
                                               Vec2{2.0f, 2.0f}),
                         Vec2{2.0f, 2.0f}));
    }

    {
        SPY_CHECK(approx(lerp(0.0f, 10.0f, 0.25f), 2.5f));
        SPY_CHECK(approx(lerp(Vec2::zero(), Vec2{10.0f, 20.0f}, 0.5f), Vec2{5.0f, 10.0f}));
        SPY_CHECK(approx(inverseLerp(0.0f, 10.0f, 2.5f), 0.25f));
        SPY_CHECK(approx(inverseLerp(1.0f, 1.0f, 5.0f), 0.0f));
        SPY_CHECK(approx(remap(5.0f, 0.0f, 10.0f, 0.0f, 100.0f), 50.0f));
        SPY_CHECK(approx(step(5.0f, 4.0f), 0.0f));
        SPY_CHECK(approx(step(5.0f, 5.0f), 1.0f));
        SPY_CHECK(approx(smoothStep(0.0f, 1.0f, 0.5f), 0.5f));
        SPY_CHECK(approx(smoothStep(0.0f, 1.0f, 0.0f), 0.0f));
        SPY_CHECK(approx(smoothStep(0.0f, 1.0f, 1.0f), 1.0f));
        SPY_CHECK(approx(smootherStep(0.0f, 1.0f, 0.5f), 0.5f));
        SPY_CHECK(approx(moveTowards(0.0f, 10.0f, 3.0f), 3.0f));
        SPY_CHECK(approx(moveTowards(8.0f, 10.0f, 5.0f), 10.0f));
        SPY_CHECK(approx(moveTowards(Vec2::zero(), Vec2{0.0f, 10.0f}, 3.0f),
                         Vec2{0.0f, 3.0f}));
    }

    {
        SPY_CHECK(approx(easeInQuad(0.0f), 0.0f));
        SPY_CHECK(approx(easeInQuad(1.0f), 1.0f));
        SPY_CHECK(approx(easeInQuad(-1.0f), 0.0f));
        SPY_CHECK(approx(easeInQuad(2.0f), 1.0f));
        SPY_CHECK(approx(easeOutQuad(0.0f), 0.0f));
        SPY_CHECK(approx(easeOutQuad(1.0f), 1.0f));
        SPY_CHECK(approx(easeInOutQuad(0.5f), 0.5f));
        SPY_CHECK(approx(easeInCubic(0.5f), 0.125f));
        SPY_CHECK(approx(easeOutCubic(0.5f), 0.875f));
        SPY_CHECK(approx(easeInSine(0.0f), 0.0f));
        SPY_CHECK(approx(easeInSine(1.0f), 1.0f));
        SPY_CHECK(approx(easeInOutSine(0.5f), 0.5f));
        SPY_CHECK(approx(easeInExpo(0.0f), 0.0f));
        SPY_CHECK(approx(easeInExpo(1.0f), 1.0f));
        SPY_CHECK(approx(easeOutExpo(1.0f), 1.0f));
        SPY_CHECK(approx(easeInOutExpo(0.0f), 0.0f));
        SPY_CHECK(approx(easeInOutExpo(1.0f), 1.0f));
        SPY_CHECK(approx(easeInBack(0.0f), 0.0f));
        SPY_CHECK(approx(easeInBack(1.0f), 1.0f));
        SPY_CHECK(approx(easeOutBack(0.0f), 0.0f));
        SPY_CHECK(approx(easeOutBack(1.0f), 1.0f));
        SPY_CHECK(approx(easeInOutBack(0.0f), 0.0f));
        SPY_CHECK(approx(easeInOutBack(1.0f), 1.0f));

        SmoothDampState state;
        float value = 0.0f;

        for (int stepIndex = 0; stepIndex < 300; ++stepIndex)
        {
            value = smoothDamp(value, 10.0f, state, 0.2f, 1.0f / 60.0f);
        }

        SPY_CHECK(approx(value, 10.0f, 1.0e-3f));
        SPY_CHECK(approx(state.velocity, 0.0f, 1.0e-2f));
        SPY_CHECK(approx(smoothDamp(3.0f, 5.0f, state, 0.2f, 0.0f), 3.0f));

        SmoothDampState immediate;
        SPY_CHECK(approx(smoothDamp(0.0f, 10.0f, immediate, 0.0f, 0.1f), 10.0f));

        SmoothDampState2D state2D;
        Vec2 value2D = Vec2::zero();

        for (int stepIndex = 0; stepIndex < 300; ++stepIndex)
        {
            value2D = smoothDamp(value2D, Vec2{10.0f, -5.0f}, state2D, 0.2f, 1.0f / 60.0f);
        }

        SPY_CHECK(approx(value2D, Vec2{10.0f, -5.0f}, 1.0e-3f));
    }

    {
        const Rect rect{Vec2{0.0f, 0.0f}, Vec2{2.0f, 2.0f}};
        const Circle circle{Vec2{0.0f, 0.0f}, 1.0f};
        const Ray2D circleRay{Vec2{-5.0f, 0.0f}, Vec2{1.0f, 0.0f}};
        const Aabb box{Vec3{-1.0f, -1.0f, -1.0f}, Vec3{1.0f, 1.0f, 1.0f}};

        SPY_CHECK(pointInRect(Vec2{0.0f, 0.0f}, rect));
        SPY_CHECK(pointInRect(Vec2{2.0f, 2.0f}, rect));
        SPY_CHECK(!pointInRect(Vec2{3.0f, 0.0f}, rect));
        SPY_CHECK(pointInCircle(Vec2{1.0f, 0.0f}, circle));
        SPY_CHECK(!pointInCircle(Vec2{1.5f, 0.0f}, circle));
        SPY_CHECK(pointInAabb(Vec3::zero(), box));

        SPY_CHECK(rectIntersectsRect(rect, Rect{Vec2{2.0f, 0.0f}, Vec2{2.0f, 2.0f}}));
        SPY_CHECK(!rectIntersectsRect(rect, Rect{Vec2{3.0f, 0.0f}, Vec2{2.0f, 2.0f}}));

        SPY_CHECK(circleIntersectsCircle(circle, Circle{Vec2{2.0f, 0.0f}, 1.0f}));
        SPY_CHECK(!circleIntersectsCircle(circle, Circle{Vec2{3.0f, 0.0f}, 1.0f}));
        SPY_CHECK(circleIntersectsRect(circle, Rect{Vec2{1.0f, 0.0f}, Vec2{1.0f, 1.0f}}));
        SPY_CHECK(!circleIntersectsRect(circle, Rect{Vec2{2.0f, 0.0f}, Vec2{1.0f, 1.0f}}));

        SPY_CHECK(aabbIntersectsAabb(box, Aabb{Vec3{1.0f, 0.0f, 0.0f}, Vec3{2.0f, 1.0f, 1.0f}}));
        SPY_CHECK(!aabbIntersectsAabb(box, Aabb{Vec3{2.0f, 0.0f, 0.0f}, Vec3{3.0f, 1.0f, 1.0f}}));

        float distance = 0.0f;
        SPY_CHECK(rayIntersectsAabb(Ray{Vec3{-5.0f, 0.0f, 0.0f}, Vec3::unitX()}, box, distance));
        SPY_CHECK(approx(distance, 4.0f));
        SPY_CHECK(rayIntersectsAabb(Ray{Vec3::zero(), Vec3::unitX()}, box, distance));
        SPY_CHECK(approx(distance, 0.0f));
        SPY_CHECK(!rayIntersectsAabb(Ray{Vec3{0.0f, 5.0f, 0.0f}, Vec3::unitX()}, box, distance));
        SPY_CHECK(!rayIntersectsAabb(Ray{Vec3{5.0f, 0.0f, 0.0f}, Vec3::unitX()}, box, distance));

        SPY_CHECK(rayIntersectsCircle(circleRay, circle, distance));
        SPY_CHECK(approx(distance, 4.0f));
        SPY_CHECK(rayIntersectsCircle(Ray2D{Vec2::zero(), Vec2::unitX()}, circle, distance));
        SPY_CHECK(approx(distance, 1.0f));
        SPY_CHECK(!rayIntersectsCircle(Ray2D{Vec2{5.0f, 0.0f}, Vec2::unitX()}, circle, distance));

        float parameter = 0.0f;
        SPY_CHECK(segmentIntersectsSegment(Vec2{0.0f, 0.0f}, Vec2{2.0f, 2.0f}, Vec2{0.0f, 2.0f},
                                           Vec2{2.0f, 0.0f}, parameter));
        SPY_CHECK(approx(parameter, 0.5f));
        SPY_CHECK(!segmentIntersectsSegment(Vec2{0.0f, 0.0f}, Vec2{1.0f, 0.0f}, Vec2{0.0f, 1.0f},
                                            Vec2{1.0f, 1.0f}, parameter));
        SPY_CHECK(!segmentIntersectsSegment(Vec2{0.0f, 0.0f}, Vec2{1.0f, 1.0f}, Vec2{2.0f, 2.0f},
                                            Vec2{3.0f, 3.0f}, parameter));
    }

    {
        const Plane plane = Plane::fromPointNormal(Vec3{0.0f, 5.0f, 0.0f}, Vec3::unitY());
        SPY_CHECK(approx(plane.signedDistance(Vec3{0.0f, 7.0f, 0.0f}), 2.0f));
        SPY_CHECK(approx(plane.signedDistance(Vec3{0.0f, 3.0f, 0.0f}), -2.0f));
        SPY_CHECK(approx(plane.projectPoint(Vec3{0.0f, 7.0f, 0.0f}), Vec3{0.0f, 5.0f, 0.0f}));

        const Plane scaledPlane{Vec3{0.0f, 2.0f, 0.0f}, -10.0f};
        SPY_CHECK(approx(scaledPlane.projectPoint(Vec3{0.0f, 8.0f, 0.0f}),
                         Vec3{0.0f, 5.0f, 0.0f}));

        const Mat4 projection = Mat4::perspective(constants::kHalfPi, 1.0f, 0.1f, 100.0f);
        const Frustum frustum = Frustum::fromViewProjection(projection);

        SPY_CHECK(frustum.containsPoint(Vec3{0.0f, 0.0f, -10.0f}));
        SPY_CHECK(!frustum.containsPoint(Vec3{0.0f, 0.0f, 10.0f}));
        SPY_CHECK(!frustum.containsPoint(Vec3{0.0f, 0.0f, -0.05f}));
        SPY_CHECK(!frustum.containsPoint(Vec3{0.0f, 0.0f, -200.0f}));
        SPY_CHECK(!frustum.containsPoint(Vec3{100.0f, 0.0f, -10.0f}));

        SPY_CHECK(frustum.intersectsSphere(Vec3{0.0f, 0.0f, -10.0f}, 1.0f));
        SPY_CHECK(!frustum.intersectsSphere(Vec3{0.0f, 0.0f, -200.0f}, 1.0f));

        SPY_CHECK(frustum.intersectsAabb(
            Aabb{Vec3{-1.0f, -1.0f, -11.0f}, Vec3{1.0f, 1.0f, -9.0f}}));
        SPY_CHECK(!frustum.intersectsAabb(
            Aabb{Vec3{100.0f, 100.0f, -11.0f}, Vec3{102.0f, 102.0f, -9.0f}}));

        const Mat4 view = Mat4::lookAt(Vec3{0.0f, 0.0f, 5.0f}, Vec3::zero(), Vec3::unitY());
        const Frustum cameraFrustum = Frustum::fromViewProjection(projection * view);
        SPY_CHECK(cameraFrustum.containsPoint(Vec3::zero()));
        SPY_CHECK(!cameraFrustum.containsPoint(Vec3{0.0f, 0.0f, 10.0f}));

        const Mat4 openGlProjection =
            Mat4::perspectiveOpenGl(constants::kHalfPi, 1.0f, 0.1f, 100.0f);
        const Frustum openGlFrustum = Frustum::fromViewProjectionOpenGl(openGlProjection);
        SPY_CHECK(openGlFrustum.containsPoint(Vec3{0.0f, 0.0f, -10.0f}));
        SPY_CHECK(!openGlFrustum.containsPoint(Vec3{0.0f, 0.0f, 10.0f}));
        SPY_CHECK(!openGlFrustum.containsPoint(Vec3{0.0f, 0.0f, -0.05f}));
        SPY_CHECK(!openGlFrustum.containsPoint(Vec3{0.0f, 0.0f, -200.0f}));
    }

    if (g_failures != 0)
    {
        std::fprintf(stderr, "%d checks failed\n", g_failures);
        return 1;
    }

    std::printf("math tests passed\n");
    return 0;
}
