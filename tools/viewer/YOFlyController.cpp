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

YOFlyController::YOFlyController(Context* context) : LogicComponent(context)
{
	pitchN_ = 0;
	rollN_= 0;
	camN_ = 0;
	frames_ = 0;
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
	enableRoll_ = true;
    input_ = GetSubsystem<Input>();
    input_->SetMouseVisible(true);
    input_->SetMouseMode(MM_FREE);
    //Orthonormalize();

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
    userFwdL_   = Vector3(1,0,0);
    userRightL_ = Vector3(0,1,0);
    userUpL_    = Vector3(0,0,1);
    //Orthonormalize();
}

void YOFlyController::SetBasis_YUp_ZForward()
{
    // Urho default
    userFwdL_   = Vector3::FORWARD;
    userRightL_ = Vector3::RIGHT;
    userUpL_    = Vector3::UP;
    //Orthonormalize();
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

Urho3D::Vector3 YOFlyController::GetRotation()
{
	Vector3 yawV = cam_.yawDeg_ * userUpL_;
	Vector3 pitchV = cam_.pitchDeg_ * userRightL_;
	Vector3 rollV = cam_.rollDeg_ * userFwdL_;
	return yawV + pitchV + rollV;
}

void YOFlyController::SetPosition(const Vector3 &pos)
{
	node_->SetPosition(pos);
}

void YOFlyController::SetRotation(const float &roll, const float &pitch, const float &yaw)
{
	cam_.yawDeg_ =  yaw;    //(userUpL_ * rotation).DotProduct(Vector3::ONE);
	cam_.pitchDeg_ = pitch; //(userRightL_ * rotation).DotProduct(Vector3::ONE);
	cam_.rollDeg_ = roll;   //(userFwdL_ *rotation).DotProduct(Vector3::ONE);
	ApplyLook(0, 0, 0);
}


void YOFlyController::GetUserAxesWorld(Vector3& fwdW, Vector3& rightW, Vector3& upW) const
{
    const Quaternion R =  camN_->GetWorldRotation() ;
    fwdW   = R * Vector3::FORWARD;
    rightW = R * Vector3::RIGHT;
    upW    = R * Vector3::UP;
}

void YOFlyController::ApplyLook(float dYaw, float dPitch, float dRoll)
{
	cam_.yawDeg_   += dYaw;
	cam_.pitchDeg_  += dPitch;
	cam_.rollDeg_  += dRoll;
    if (node_)     node_->SetRotation(  Quaternion(cam_.yawDeg_,   userUpL_));    // Z (yaw)
    if (pitchN_)   pitchN_->SetRotation(Quaternion(cam_.pitchDeg_, userRightL_)); // Y (pitch)
    if (rollN_)    rollN_->SetRotation( Quaternion(cam_.rollDeg_,  Vector3::FORWARD));   // X (roll)
}

void YOFlyController::MoveTo(int frames, Vector3 pos, float yawDeg, float pitchDeg, float rollDeg, float fov)
{
	frames_ = frames;
	Vector3 curRot = GetRotation();

	camDelta_.fov_ = (camera_->GetFov() - fov)/ frames;

	camDelta_.pitchDeg_ = (pitchDeg - cam_.pitchDeg_)/ frames;
	camDelta_.yawDeg_ = (yawDeg - cam_.yawDeg_ )/ frames;
	camDelta_.rollDeg_ = (rollDeg - cam_.rollDeg_)/ frames;

	std::cout << " DELTA "<< camDelta_.yawDeg_ << " " << camDelta_.pitchDeg_  << " " << camDelta_.rollDeg_ << std::endl;

	posDelta_ = (pos - node_->GetPosition()) / frames;
}

void YOFlyController::SetCamera(Urho3D::SharedPtr<Urho3D::Camera> camera)
{
    pitchN_ = node_->CreateChild("Pitch");
    rollN_= pitchN_->CreateChild("Roll");
    camN_ = rollN_->CreateChild("Camera");
	SetBasis_ZUp_XForward();
	camN_->LookAt(camN_->GetWorldPosition() + userUpL_ * Vector3(1,1,-1), userFwdL_, TS_WORLD);
	camera_ = camera;
	camN_->AddComponent(camera_, 255);
}

static Plane xyPlane( Vector3::FORWARD,  Vector3::ZERO);

void YOFlyController::Update(float dt)
{
    if (!input_)
    	return;

    IntVector2 md = input_->GetMouseMove();

    if (input_->GetMouseButtonDown(MOUSEB_LEFT) && md != IntVector2::ZERO)
    {
        IntVector2 p0 = GetSubsystem<Input>()->GetMousePosition();
        IntVector2 p1 = p0 + md;
        auto* graphics = GetSubsystem<Graphics>();

        float n0x = (float)p0.x_ / graphics->GetWidth();
        float n0y = (float)p0.y_ / graphics->GetHeight();
        Ray r0 = camera_->GetScreenRay(n0x, n0y);
        float d0 = r0.HitDistance(xyPlane);
        Vector3 out0 = r0.origin_ + r0.direction_ * d0;

        float n1x = (float)p1.x_ / graphics->GetWidth();
        float n1y = (float)p1.y_ / graphics->GetHeight();
        Ray r1 = camera_->GetScreenRay(n1x, n1y);
        float d1 = r1.HitDistance(xyPlane);
        Vector3 out1 = r1.origin_ + r1.direction_ * d1;

        //printf("%d %d MOVE from (%f, %f, %f) to (%f, %f, %f) \n", md.x_, md.y_, out0.x_, out0.y_, out0.z_, out1.x_, out1.y_, out1.z_ );
        node_->Translate(out0 - out1, TS_WORLD);
    }

    if(Abs(md.x_) > Abs(md.y_))
        md.y_ = 0.0f;
    else if(Abs(md.x_) < Abs(md.y_))
        md.x_ = 0.0f;
    // --- Mouse look (RMB held)
    if (input_->GetMouseButtonDown(MOUSEB_RIGHT))
    {
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
    if (input_->GetKeyDown(KEY_SHIFT))
    	speed *= fastMul_;
    else if (input_->GetKeyDown(KEY_CTRL))
    	speed *= slowMul_;

    Vector3 fwdW, rightW, upW;
    GetUserAxesWorld(fwdW, rightW, upW);

    Vector3 move;
    if (((input_->GetMouseButtonDown(MOUSEB_MIDDLE) && md.y_ > 0) || input_->GetScancodeDown(SCANCODE_W) )) move += fwdW;
    if (((input_->GetMouseButtonDown(MOUSEB_MIDDLE) && md.y_ < 0) || input_->GetScancodeDown(SCANCODE_S) )) move -= fwdW;
    if (input_->GetScancodeDown(SCANCODE_D)) move += rightW;
    if (input_->GetScancodeDown(SCANCODE_A)) move -= rightW;
    if (input_->GetScancodeDown(SCANCODE_E)) move += upW;
    if (input_->GetScancodeDown(SCANCODE_Q)) move -= upW;

    if (move.LengthSquared() > 0.0f)
        node_->Translate(move.Normalized() * speed * dt, TS_WORLD);

    if(frames_ > 0)
    {
    	node_->Translate(posDelta_, TS_WORLD);
    	ApplyLook(camDelta_.yawDeg_, camDelta_.pitchDeg_, camDelta_.rollDeg_);
    	camera_->SetFov(camera_->GetFov()  - camDelta_.fov_);
    	frames_--;
    }


}
