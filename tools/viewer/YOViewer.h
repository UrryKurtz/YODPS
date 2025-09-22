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

#define YO_LMASK_WORLD (0x0001)
#define YO_LMASK_OVERLAY (0x0010)

using namespace Urho3D;

struct YOInputData
{
	Node* root;
	std::array<std::vector<Node*>, YO_TYPE_NUM> types;
	std::array<std::vector<CustomGeometry*>, YO_TYPE_NUM> geom_lines;
	std::array<std::vector<CustomGeometry*>, YO_TYPE_NUM> geom_fills;
	std::array<std::vector<CustomGeometry*>, YO_TYPE_NUM> geoms;
};

class YOViewer: public Application
{
URHO3D_OBJECT(YOViewer, Application)

pthread_t thread_inputs_{};
pthread_t thread_videos_{};

public:
    YOViewer(Context *context) : Application(context)
    {
        exit_ = false;
        cache_ = GetSubsystem<ResourceCache>();
        camera_select_ = 0; //TODO read from config
    }
    virtual ~YOViewer()
    {
        pthread_join(thread_inputs_, NULL);
        pthread_join(thread_videos_, NULL);
    }
    void Setup() override;
    void Start() override;
    void Stop() override
    {
        std::cout << " STOP " << std::endl;
        pthread_cancel(thread_inputs_);
        pthread_cancel(thread_videos_);
    }
    void AddFrame(std::shared_ptr<YOVariant> frame, int frame_id);
    void AddVideo(SharedPtr<Image> frame, int frame_id);

    void CreateMaterial(int input, int type, YOVariant &style);
    YOVariant *GetConfig(YOVariant  &config, const std::string &path);
    void RegisterTopic(const std::string &name, int num);
    int GetTopicId(const std::string &name);

private:

    ResourceCache *cache_;
    bool exit_;
    std::shared_ptr<YOVariant> config_;
    std::shared_ptr<YOGui> gui_;

    SharedPtr<Scene>scene_;

    //SharedPtr<Material> materials_[YO_INPUT_NUM][YO_TYPE_NUM] = {0};
    //bool materials_update[YO_INPUT_NUM][YO_TYPE_NUM] = {false};

    SharedPtr<Node>world_;
    SharedPtr<Node>world_geom_;

    SharedPtr<Node>overlay_;
    SharedPtr<Node>overlay_geom_;

    SharedPtr<Node>internal_;

    SharedPtr<Node>grid_;

    int camera_select_;
    SharedPtr<Node>cameraNode_;
    SharedPtr<Camera>camera_;
    SharedPtr<YOFlyController>controller_;

    SharedPtr<Technique> technique_;
    SharedPtr<Technique> technique_overlay_;

    SharedPtr<Texture2D> texture_line_;
    SharedPtr<Material> material_line_;

    SharedPtr<Texture2D> texture_param_;
    SharedPtr<Material> material_param_;

    SharedPtr<Texture2D> texture_fill_;
    SharedPtr<Material> material_fill_;

    SharedPtr<Texture2D> texture_text_;
    SharedPtr<Material> material_text_;

    std::array<std::shared_ptr<YOVariant>, YO_INPUT_NUM> data_in_;
    std::array<std::shared_ptr<YOInputData>, YO_INPUT_NUM> data_;
    std::array<std::mutex, YO_INPUT_NUM> data_lock_;

    std::array<SharedPtr<Texture2D>, YO_VIDEO_NUM> video_;
    std::array<std::mutex, YO_VIDEO_NUM> video_lock_;
    std::array<SharedPtr<Sprite>, YO_VIDEO_NUM> video_pad_;
    std::array<SharedPtr<Image>, YO_VIDEO_NUM> video_img_;

    std::map<std::string, int> topics_;

    //std::array<Node*, YO_INPUT_NUM> data_;

    void CreateCamera();
    void CreateScene();
    void CreateLight();
    SharedPtr<Texture2D> CreateTexture();
    void CreateTextures();
    void CreateConfig();
    void CreateSprite(SharedPtr<Texture2D> tex, Vector2 pos);

    std::shared_ptr<YOInputData> ConvertFrame(std::shared_ptr<YOVariant>, int id);
    void ConvertGeometry(YOVariant &obj, std::shared_ptr<YOInputData> fdata, YOVariant &input_cfg, int frame_id);

    void ConvertVideos();

    void CreateXYGrid(Node *parent, int cellsX = 10, int cellsY = 10, float spacing = 1.0f, float z = 0.0f);

    void HandleKeyDown(StringHash eventType, VariantMap& eventData);
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    void RenderUI();
    void ProcessChange();
};
