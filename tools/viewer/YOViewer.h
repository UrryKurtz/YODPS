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
#include <Graphics/CustomGeometry.h>
#include <Graphics/Graphics.h>
#include <Graphics/Model.h>
#include <Graphics/Octree.h>
#include <Graphics/Renderer.h>
#include <Graphics/StaticModel.h>
#include <Graphics/Texture2D.h>
#include <Graphics/Viewport.h>
#include <Graphics/Zone.h>
#include <Resource/ResourceCache.h>
#include <Scene/Node.h>
#include <Scene/Scene.h>
#include <Scene/Node.h>
#include <UI/Sprite.h>

#include "YOFlyController.h"
#include "YOGui.h"

#include "YONode.h"
#include "YOVariant.h"
#include <queue>
#include <array>

#include <GL/gl.h>

#include <pthread.h>
#include "YOPluginBus.h"


#define YO_LMASK_WORLD (0x0001)
#define YO_LMASK_OVERLAY (0x0010)

using namespace Urho3D;

class YOViewer: public Application
{
URHO3D_OBJECT(YOViewer, Application)

YOPluginBus *plugin_bus_;

public:
    YOViewer(Context *context) : Application(context)
    {
        exit_ = false;
        cache_ = GetSubsystem<ResourceCache>();
        plugin_bus_ = new YOPluginBus(context_);
    }
    virtual ~YOViewer()
    {
    }
    void Setup() override;
    void Start() override;
    void Stop() override
    {
        std::cout << " STOP " << std::endl;
    }
    void AddVideo(SharedPtr<Image> frame, int frame_id);

private:

    ResourceCache *cache_;
    bool exit_;
    std::shared_ptr<YOVariant> config_;
    YOGui gui_;

    SharedPtr<Scene>scene_;
    SharedPtr<Node>world_;
    SharedPtr<Node>overlay_;
    SharedPtr<Node>internal_;
    SharedPtr<Node>grid_;

    //int camera_select_;
    SharedPtr<Node>cameraNode_;
    SharedPtr<Camera>camera_;
    SharedPtr<YOFlyController>controller_;

    SharedPtr<Technique> technique_;
    SharedPtr<Technique> technique_overlay_;

    void CreateCamera();
    void CreateScene();
    void CreateLight();

    void CreateConfig(std::string fileName = "config.xml");
    void CreateSprite(SharedPtr<Texture2D> tex, Vector2 pos);

    void CreateXYGrid(Node *parent, int cellsX = 10, int cellsY = 10, float spacing = 1.0f, float z = 0.0f);

    void HandleKeyDown(StringHash eventType, VariantMap& eventData);
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    void HandleFrame(StringHash eventType, VariantMap& eventData);
    void RenderUI();
};
