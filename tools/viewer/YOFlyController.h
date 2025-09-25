/*
 * YOFlyController.h
 *
 *  Created on: Aug 31, 2025
 *      Author: kurtz
 */
#pragma once
#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Graphics/Camera.h>


namespace Urho3D {
class Input;
}

using namespace Urho3D;

struct CamInfo{
    float fov_;
    float yawDeg_ = 0.f;
	float pitchDeg_ = 0.f;
	float rollDeg_ = 0.f;
};


/// Free-fly camera/controller with configurable user basis (Forward/Right/Up).
/// Engine basis is (+X Right, +Y Up, +Z Forward). You can remap which engine axis
/// represents user Forward/Right/Up (e.g., user Up = engine +Z).
class YOFlyController : public LogicComponent
{
    URHO3D_OBJECT(YOFlyController, LogicComponent);

    SharedPtr<Camera> camera_;

    Node* pitchN_;
    Node* rollN_;
    Node* camN_;

    CamInfo cam_;
    CamInfo camDelta_;
    Vector3 posDelta_;

    int frames_;

public:
    explicit YOFlyController(Context* context);

    static void RegisterObject(Context* context);

    // --- Tuning ---
    void SetMouseSensitivity(float degPerPixel) { mouseSens_ = degPerPixel; }
    void SetMoveSpeed(float unitsPerSec)       { moveSpeed_ = unitsPerSec; }
    void SetSpeedMul(float slowMul, float fastMul) { slowMul_ = slowMul; fastMul_ = fastMul; }
    void SetInvertY(bool inv)                  { invertY_ = inv; }
    void EnableRoll(bool enable)               { enableRoll_ = enable; }
    Vector3 GetRotation();
    const Vector3& GetPosition();
    void SetRotation(const float &roll, const float &pitch, const float &yaw);
    void SetPosition(const Vector3 &pos);

    void SetCamera(SharedPtr<Camera> camera);

    // Pitch clamp in degrees (symmetric ±limit). Use >= 0.0f; set to 0 to disable clamp.
    void SetPitchClamp(float degrees)          { pitchClampDeg_ = degrees; }

    // Set user basis using ENGINE-LOCAL unit vectors that represent user axes.
    // Vectors don't need to be perfectly orthonormal; they will be normalized and re-orthogonalized.
    // Example for Z-up, X-forward (automotive):
    //   SetUserBasis(Vector3::RIGHT /*F*/, Vector3::UP /*R*/, Vector3::FORWARD /*U*/);
    void SetUserBasis(const Vector3& forward, const Vector3& right, const Vector3& up);

    // Optional: preset helpers
    void SetBasis_ZUp_XForward();     // Forward=+X, Right=+Y, Up=+Z
    void SetBasis_YUp_ZForward();     // Urho default (Forward=+Z, Right=+X, Up=+Y)

    // Apply yaw/pitch/roll respecting user basis and optional clamping
    void ApplyLook(float dyawDeg, float dpitchDeg, float drollDeg);

    void MoveTo(int frames, Vector3 pos, float dyawDeg, float dpitchDeg, float drollDeg, float fov);


protected:
    void Start() override;
    void Update(float dt) override;
    void OnSetAttribute(const AttributeInfo& attr, const Variant& value) override;


private:
    // Build an orthonormal basis from arbitrary 3 vectors
    void Orthonormalize();

    // Get current user-space axes in WORLD coordinates
    void GetUserAxesWorld(Vector3& fwdW, Vector3& rightW, Vector3& upW) const;

    SharedPtr<Input> input_;

    // User basis in ENGINE-LOCAL sparho3D::
    Vector3 userFwdL_{ Vector3::FORWARD }; // default = engine +Z
    Vector3 userRightL_{ Vector3::RIGHT }; // default = engine +X
    Vector3 userUpL_{ Vector3::UP };       // default = engine +Y

    float mouseSens_{ 0.2f };   // degrees per pixel
    float moveSpeed_{ 8.0f };   // units per second
    float slowMul_{ 0.5f }, fastMul_{ 5.0f };
    bool  invertY_{ false };
    bool  enableRoll_{ false }; // hold Alt to roll (or bind differently below)
    float pitchClampDeg_{ 85.0f }; // 0 = disabled

    // Accumulated pitch around user Right axis (for clamping)
    float pitchAccumDeg_{ 0.0f };
};
