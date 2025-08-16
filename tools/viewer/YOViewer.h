//
// Copyright (c) 2008-2022 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#pragma once

#include <Core/CoreEvents.h>
#include <Core/Context.h>
#include <Core/Main.h>
#include <Engine/Application.h>
#include <Engine/Engine.h>
#include <Engine/EngineDefs.h>
#include <Input/Input.h>
#include <Input/InputEvents.h>
#include <Input/FreeFlyController.h>
#include <Graphics/Camera.h>
#include <Graphics/Model.h>
#include <Graphics/Octree.h>
#include <Graphics/Renderer.h>
#include <Graphics/StaticModel.h>
#include <Graphics/Viewport.h>
#include <Graphics/Zone.h>
#include <Resource/ResourceCache.h>

#include <Scene/Scene.h>
#include <Scene/Node.h>

using namespace Urho3D;

class YOViewer: public Application
{
URHO3D_OBJECT(YOViewer, Application)
    ;

public:
    YOViewer(Context *context) : Application(context)
    {
    }

    void Setup() override
    {
        engineParameters_[EP_WINDOW_TITLE] = "YOViewer";
        engineParameters_[EP_FULL_SCREEN] = false;
    }

    void Start() override
    {
        CreateScene();
        CreateCamera();

        // Вьюпорт
        auto *renderer = GetSubsystem<Renderer>();
        SharedPtr<Viewport>viewport(new Viewport(context_, scene_, cameraNode_->GetComponent<Camera>()));
        renderer->SetViewport(0, viewport);

        // Подписка на клавиши
        SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(YOViewer, HandleKeyDown));
    }

private:
    SharedPtr<Scene>scene_;
    SharedPtr<Node>world_;
    SharedPtr<Node>cameraNode_;
    SharedPtr<Camera>camera_;
    SharedPtr<FreeFlyController>controller_;

    void CreateScene()
    {
        scene_ = new Scene(context_);
        scene_->CreateComponent<Octree>();

        world_ = scene_->CreateChild("WorldRoot");
        world_->SetRotation(Quaternion(0, -90, 0));

        // Зона (фон, освещение)
        Node *zoneNode = world_->CreateChild("Zone");
        auto *zone = zoneNode->CreateComponent<Zone>();
        zone->SetAmbientColor(Color(0.3f, 0.3f, 0.3f));
        zone->SetFogColor(Color(0.5f, 0.5f, 0.7f));
        zone->SetFogStart(50.0f);
        zone->SetFogEnd(200.0f);
        zone->SetBoundingBox(BoundingBox(-1000.0f, 1000.0f));

        // Направленный свет (как солнце)
        Node *lightNode = world_->CreateChild("DirectionalLight");
        lightNode->SetDirection(Vector3(0.5f, -1.0f, 0.5f));
        auto *light = lightNode->CreateComponent<Light>();
        light->SetLightType(LIGHT_DIRECTIONAL);
        light->SetBrightness(1.2f);

        // Несколько кубиков
        ResourceCache *cache = GetSubsystem<ResourceCache>();

        for (int i = 0; i < 5; ++i)
        {
            Node *boxNode = world_->CreateChild("Box");
            boxNode->SetPosition(Vector3(i * 2.0f, 0.0f, 0.0f));
            boxNode->SetScale(1.0f);

            auto *boxModel = boxNode->CreateComponent<StaticModel>();
            boxModel->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
            boxModel->SetMaterial(cache->GetResource<Material>("Materials/Stone.xml"));
        }
    }

    void CreateCamera()
    {
        // Камера
        cameraNode_ = world_->CreateChild("Camera");
        camera_ = cameraNode_->CreateComponent<Camera>();

        // FreeFly контрол
        controller_ = cameraNode_->CreateComponent<FreeFlyController>();
        controller_->SetSpeed(10.0f);

        //controller->SetRotationSpeed(0.2f);
    }

    void HandleKeyDown(StringHash, VariantMap &eventData)
    {
        using namespace KeyDown;
        if (eventData[P_KEY].GetInt() == KEY_ESCAPE)
            engine_->Exit();

        printf("cam pos: (%0.02f, %0.02f, %0.02f)\n", cameraNode_->GetPosition().x_, cameraNode_->GetPosition().y_, cameraNode_->GetPosition().z_);
    }
};
