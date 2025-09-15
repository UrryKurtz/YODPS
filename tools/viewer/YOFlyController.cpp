/*
 * YOFlyController.cpp
 *
 *  Created on: Aug 31, 2025
 *      Author: kurtz
 */

#include "YOFlyController.h"
#include <Urho3D/Core/Attribute.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Math/Plane.h>
#include <Urho3D/Math/Ray.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Graphics.h>

using namespace Urho3D;

YOFlyController::YOFlyController(Context* context) : LogicComponent(context)
{
    SetUpdateEventMask(USE_UPDATE);
}

void YOFlyController::RegisterObject(Context* context)
{
    context->RegisterFactory<YOFlyController>();
    // Editable/serializable attributes
    URHO3D_ATTRIBUTE("Mouse Sensitivity", float, mouseSens_, 1.0f, AM_DEFAULT);
    URHO3D_ATTRIBUTE("Move Speed", float, moveSpeed_, 8.0f, AM_DEFAULT);
    URHO3D_ATTRIBUTE("Slow Multiplier", float, slowMul_, 0.3f, AM_DEFAULT);
    URHO3D_ATTRIBUTE("Fast Multiplier", float, fastMul_, 4.0f, AM_DEFAULT);
    URHO3D_ATTRIBUTE("Invert Y", bool, invertY_, false, AM_DEFAULT);
    URHO3D_ATTRIBUTE("Enable Roll", bool, enableRoll_, false, AM_DEFAULT);
    URHO3D_ATTRIBUTE("Pitch Clamp Deg", float, pitchClampDeg_, 85.0f, AM_DEFAULT);

    // Basis vectors (engine-local). Will be orthonormalized at Start/OnSetAttribute.
    URHO3D_ATTRIBUTE("User Forward L", Vector3, userFwdL_,   Vector3::FORWARD, AM_DEFAULT);
    URHO3D_ATTRIBUTE("User Right L",   Vector3, userRightL_, Vector3::RIGHT,   AM_DEFAULT);
    URHO3D_ATTRIBUTE("User Up L",      Vector3, userUpL_,    Vector3::UP,      AM_DEFAULT);
}

void YOFlyController::Start()
{
    input_ = GetSubsystem<Input>();
    input_->SetMouseVisible(true);
    input_->SetMouseMode(MM_FREE);
    Orthonormalize();
    pitchAccumDeg_ = 0.0f;
}

void YOFlyController::SetUserBasis(const Vector3& forward, const Vector3& right, const Vector3& up)
{
    userFwdL_   = forward;
    userRightL_ = right;
    userUpL_    = up;
    Orthonormalize();
}

void YOFlyController::SetBasis_ZUp_XForward()
{
    // Forward=+X, Right=+Y, Up=+Z (automotive-friendly)
    userFwdL_   = Vector3::RIGHT;
    userRightL_ = Vector3::UP;
    userUpL_    = Vector3::FORWARD;
    Orthonormalize();
}

void YOFlyController::SetBasis_YUp_ZForward()
{
    // Urho default
    userFwdL_   = Vector3::FORWARD;
    userRightL_ = Vector3::RIGHT;
    userUpL_    = Vector3::UP;
    Orthonormalize();
}

void YOFlyController::OnSetAttribute(const AttributeInfo& attr, const Variant& value)
{
    // Keep base behavior
    LogicComponent::OnSetAttribute(attr, value);

    // Re-orthonormalize basis when any of the basis attributes change
    if (attr.name_ == "User Forward L" ||
        attr.name_ == "User Right L"  ||
        attr.name_ == "User Up L")
    {
        Orthonormalize();
    }
}

void YOFlyController::Orthonormalize()
{
    // Gram-Schmidt to build a right-handed orthonormal basis
    Vector3 f = userFwdL_.Normalized();
    Vector3 u = userUpL_.Normalized();

    // Make 'f' orthogonal to 'u'
    f = (f - u * f.DotProduct(u)).Normalized();

    // Right = u x f (right-handed)
    Vector3 r = u.CrossProduct(f).Normalized();

    // Recompute up to ensure exact orthogonality
    u = f.CrossProduct(r).Normalized();

    userFwdL_ = f;
    userRightL_ = r;
    userUpL_ = u;

    // Reset pitch accumulator to avoid sudden clamp jumps after basis change
    pitchAccumDeg_ = 0.0f;
}

void YOFlyController::GetUserAxesWorld(Vector3& fwdW, Vector3& rightW, Vector3& upW) const
{
    const Quaternion R = node_->GetWorldRotation();
    fwdW   = R * userFwdL_;
    rightW = R * userRightL_;
    upW    = R * userUpL_;
}

void YOFlyController::ApplyLook(float dyawDeg, float dpitchDeg, float drollDeg)
{
    Vector3 fwdW, rightW, upW;
    GetUserAxesWorld(fwdW, rightW, upW);

    // Yaw around user Up (world-space)
    if (Abs(dyawDeg) > 0.0f)
    {
        node_->Rotate(Quaternion(dyawDeg, Vector3(0,0,1) ), TS_WORLD);   //Vector3(1,0,0) upW userUpL_
    }

    // Pitch around user Right (world-space) with optional clamp
    if (Abs(dpitchDeg) > 0.0f)
    {
        node_->Rotate(Quaternion(dpitchDeg, rightW), TS_WORLD);
    }

    // Optional roll around Forward (world-space)
    if (enableRoll_ && Abs(drollDeg) > 0.0f)
    {
        node_->Rotate(Quaternion(drollDeg, fwdW), TS_WORLD);
    }
}

void YOFlyController::Update(float dt)
{
    if (!input_) return;
    IntVector2 md = input_->GetMouseMove();

    static Plane xyPlane( Vector3::FORWARD,  Vector3::ZERO);

    if (input_->GetMouseButtonDown(MOUSEB_LEFT) && md != IntVector2::ZERO)
    {
        IntVector2 p0 = GetSubsystem<Input>()->GetMousePosition();
        IntVector2 p1 = p0 + md;
        auto *camera = node_->GetComponent<Camera>();
        auto* graphics = GetSubsystem<Graphics>();

        float n0x = (float)p0.x_ / graphics->GetWidth();
        float n0y = (float)p0.y_ / graphics->GetHeight();
        Ray r0 = camera->GetScreenRay(n0x, n0y);
        float d0 = r0.HitDistance(xyPlane);
        Vector3 out0 = r0.origin_ + r0.direction_ * d0;

        float n1x = (float)p1.x_ / graphics->GetWidth();
        float n1y = (float)p1.y_ / graphics->GetHeight();
        Ray r1 = camera->GetScreenRay(n1x, n1y);
        float d1 = r1.HitDistance(xyPlane);
        Vector3 out1 = r1.origin_ + r1.direction_ * d1;

        //printf("%d %d MOVE from (%f, %f, %f) to (%f, %f, %f) \n", md.x_, md.y_, out0.x_, out0.y_, out0.z_, out1.x_, out1.y_, out1.z_ );
        node_->Translate(out0 - out1, TS_WORLD);
    }
    // --- Mouse look (RMB held)
    if (input_->GetMouseButtonDown(MOUSEB_RIGHT))
    {


//        if(md.x_ != 0.0f && md.y_ != 0.0f)
//        {
//            printf ("DIRECTION (%.02f, %.02f,%.02f)\n", node_->GetWorldDirection().x_, node_->GetWorldDirection().y_, node_->GetWorldDirection().z_);
//        }

        if(Abs(md.x_) > Abs(md.y_))
            md.y_ = 0.0f;
        else if(Abs(md.x_) < Abs(md.y_))
            md.x_ = 0.0f;

        float dyaw   = -md.x_ * mouseSens_;                      // horizontal → yaw
        float dpitch = (invertY_ ? md.y_ : -md.y_) * mouseSens_; // vertical → pitch
        float droll  = 0.0f;

        // Optional: hold ALT to roll with horizontal mouse motion
        if (enableRoll_ && (input_->GetKeyDown(KEY_LALT) || input_->GetKeyDown(KEY_RALT)))
        {
            droll = dyaw;
            dyaw  = 0.0f;
        }
        ApplyLook(dyaw, dpitch, droll);
    }

    // --- Movement (WASD + QE)
    float speed = moveSpeed_;
    if (input_->GetKeyDown(KEY_SHIFT)) speed *= fastMul_;
    else if (input_->GetKeyDown(KEY_CTRL)) speed *= slowMul_;

    Vector3 fwdW, rightW, upW;
    GetUserAxesWorld(fwdW, rightW, upW);

    Vector3 move;
    if (input_->GetScancodeDown(SCANCODE_W)) move += fwdW;
    if (input_->GetScancodeDown(SCANCODE_S)) move -= fwdW;
    if (input_->GetScancodeDown(SCANCODE_D)) move += rightW;
    if (input_->GetScancodeDown(SCANCODE_A)) move -= rightW;
    if (input_->GetScancodeDown(SCANCODE_E)) move += upW;
    if (input_->GetScancodeDown(SCANCODE_Q)) move -= upW;

    if (move.LengthSquared() > 0.0f)
        node_->Translate(move.Normalized() * speed * dt, TS_WORLD);
}
