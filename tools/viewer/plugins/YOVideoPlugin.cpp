/*
 * YOVideoPlugin.cpp
 *
 *  Created on: Sep 23, 2025
 *      Author: kurtz
 */

#include <Urho3D/UI/UI.h>
#include "YOVideoPlugin.h"

#include <turbojpeg.h>

inline bool DecodeJpegToRGBA(const uint8_t* data, size_t size, SharedPtr<Image> rgba, int& width, int& height)
{
    tjhandle tj = tjInitDecompress();
    if (!tj)
    	return false;

    int subsamp = 0, colorspace = 0;
    if (tjDecompressHeader3(tj, data, (unsigned long)size, &width, &height, &subsamp, &colorspace) != 0)
    {
    	tjDestroy(tj);
    	return false;
    }
    rgba->SetSize(width,height, 4);
    const int flags = TJFLAG_FASTDCT; //TJFLAG_ACCURATEDCT; //TJFLAG_FASTDCT;
    if (tjDecompress2(tj, data, (unsigned long)size, rgba->GetData(), width, 0, height, TJPF_RGBA, flags) != 0)
    {
    	tjDestroy(tj);
    	return false;
    }
    tjDestroy(tj);
    return true;
}

void YOVideoPlugin::OnData(const std::string &topic, std::shared_ptr<YOMessage> message)
{
	YOImageData *img_info = (YOImageData *)message->getData();
	if(img_info->format == YOFrameFormat::YO_JPEG)
	{
		int width, height;
		auto rgba = MakeShared<Image>(context_);
		if(DecodeJpegToRGBA(message->getExtData(), message->getExtDataSize(), rgba, width, height))
		{
			AddVideo(rgba, GetTopicId(topic));
		}
	}
}

YOVideoPlugin::YOVideoPlugin(Context* context) : IPlugin (context)
{

}

YOVideoPlugin::~YOVideoPlugin() {

}

void YOVideoPlugin::OnGui()
{
	int start = config_->m_name.length() + 1;

	if(gui_.draw(*config_))
	{
		std::string path = gui_.getPath().substr(start);
		YOVariant *res = gui_.getConfig();
		std::vector<int> addr = gui_.getIndex();
		std::cout << "GUI changed: " <<  path << std::endl;

		if(path == "/video/#0/enabled")
		{
			video_pad_[addr[0]]->SetVisible(res->get<bool>());
		}
		else if(path == "/video/#0/transform/position")
		{
			YOVector3 &pos = res->get<YOVector3>();
			std::cout << "POS: " <<  pos << std::endl;
			video_pad_[addr[0]]->SetPosition(pos.x, pos.y);
		}
		else if(ends_with(path, "/video/#0/transform/scale"))
		{
			YOVector3 pos = res->get<YOVector3>();
			video_pad_[addr[0]]->SetScale(pos.x, pos.y);
		}
		else if(ends_with(path, "/video/#0/transform/rotation"))
		{
			YOVector3 pos = res->get<YOVector3>();
			video_pad_[addr[0]]->SetRotation(pos.z);
		}
		else if(ends_with(path, "/video/#0/opacity"))
		{
			YOLimitF op = res->get<YOLimitF>();
			video_pad_[addr[0]]->SetOpacity(op.value);
		}
	}
}

void YOVideoPlugin::OnStart()
{
	video_cfg_ = &(*config_)[yo::k::video];
	params_cfg_ = &(*config_)[yo::k::config];

	if(video_cfg_->getTypeId() != 1 || video_cfg_->getArraySize() != YO_VIDEO_NUM )
	{
		std::cout << config_->m_name << " Plugin. Create default config "<< std::endl;
	    video_cfg_->m_value = YOArray(YO_VIDEO_NUM);
	    for(int i = 0; i < YO_VIDEO_NUM; i++)
	    {
	        createVideoCfg(i, (*video_cfg_)[i], yo::k::video);
	    }
	}

	for(int i = 0; i < YO_VIDEO_NUM; i++)
	{
		std::string topic = "VIDEO" + std::to_string(i);
		RegisterTopic(topic, i);
	}

	CreateTextures();
}

void YOVideoPlugin::CreateTextures()
{
    auto* ui = GetSubsystem<UI>();
	auto* root = ui->GetRoot();

	YOVariant &video_cfg = config_->get(yo::k::video);
	int idx = 0;
	for(auto &v : video_)
	{
		v = MakeShared<Texture2D>(context_);
		video_img_[idx] = MakeShared<Image>(context_);

		YOVariant &cur_vid = video_cfg[idx];
		YOVariant &transform = cur_vid[yo::k::transform];
		YOVector3 &pos = transform[yo::k::position];
		YOVector3 &scale = transform[yo::k::scale];

		video_pad_[idx] = root->CreateChild<Sprite>();
		video_pad_[idx]->SetTexture(v);
		video_pad_[idx]->SetSize(v->GetSize());
		video_pad_[idx]->SetBlendMode(BLEND_ALPHA);
		video_pad_[idx]->SetPosition(pos.x, pos.y);
		video_pad_[idx]->SetScale(scale.x, scale.y);
		idx++;
	}
}

void YOVideoPlugin::OnUpdate(float timeStep)
{
	ConvertVideos();
}

void YOVideoPlugin::AddVideo(SharedPtr<Image> frame, int frame_id)
{
    video_lock_[frame_id].lock();
    video_img_[frame_id] = frame;
    video_lock_[frame_id].unlock();
}

void YOVideoPlugin::ConvertVideos()
{
	for(int i = 0; i < YO_VIDEO_NUM; i++)
	{
		video_lock_[i].lock();
		SharedPtr<Image> img = video_img_[i];
		video_img_[i].Reset();
		if(img)
		{
			video_[i]->SetData(img);
			if(img->GetWidth() != video_pad_[i]->GetWidth() || img->GetHeight() != video_pad_[i]->GetHeight())
			{
				 std::cout << "Resize img " << i << " " << video_[i]->GetWidth() << " " << video_[i]->GetHeight() << std::endl;
				 video_pad_[i]->SetSize(img->GetSize().x_, img->GetSize().y_);
				 video_pad_[i]->SetHotSpot(img->GetSize().x_/2, img->GetSize().y_/2);
				 video_pad_[i]->SetTexture(video_[i]);
			}
		}
		video_lock_[i].unlock();
	}
}
