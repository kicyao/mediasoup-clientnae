/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#ifndef GALLERY_VIEW_H
#define GALLERY_VIEW_H

#include <QFrame>
#include <vector>
#include <algorithm>
#include "api/media_stream_interface.h"
#include "mac_video_renderer.h"

namespace Ui {
class GalleryView;
}

class QGridLayout;

namespace webrtc {
	class VideoTrackInterface;
}

class IContentView {
public:
	virtual ~IContentView() {}
	virtual bool init() = 0;
	virtual void cleanup() = 0;
    virtual const std::string& id() = 0;
	virtual QWidget* view() = 0;
};

class ContentView : public IContentView {

public:
    ContentView(const std::string& id, webrtc::VideoTrackInterface* track, MacVideoRenderer* renderer)
	: _id(id)
	, _track(track)
	, _renderer(renderer) {

	}
	bool init() override {
		if (_renderer && _track) {
			rtc::VideoSinkWants wants;
			_track->AddOrUpdateSink(_renderer, wants);
			return true;
		}
		return false;
	}

	void cleanup() override {
		if (_renderer && _track) {
			_track->RemoveSink(_renderer);
            _track = nullptr;
            _renderer->destroy();
		}
	}

    const std::string& id() override {
		return _id;
	}

	QWidget* view() override {
		return static_cast<QWidget*>(_renderer);
	}

private:
    std::string _id;

    webrtc::VideoTrackInterface* _track = nullptr;

    MacVideoRenderer* _renderer = nullptr;
};

class PermuteStrategy {
public:
    virtual ~PermuteStrategy() {}

    virtual void permute(std::vector<std::shared_ptr<IContentView>>& views) = 0;
};

class DefaultStrategy : public PermuteStrategy {
public:
    DefaultStrategy() {}

    void permute(std::vector<std::shared_ptr<IContentView>>& views) override
    {
        std::sort(views.begin(), views.end(), [](const auto& e1, const auto& e2){
            return e1->id() < e2->id();
        });
    }
};

class GalleryView : public QFrame
{
    Q_OBJECT

    enum class Strategy : int {
      DEFAULT = 0
    };

public:
    explicit GalleryView(QWidget *parent = nullptr);

    ~GalleryView();

    void insertView(std::shared_ptr<ContentView> view);

    void removeView(const std::string& id);

    QWidget* getView(const std::string& id);

    void removeAll();

protected:
    void init();

    std::shared_ptr<PermuteStrategy> getPermuteStrategy(Strategy strategys);

    void permute(Strategy strategy = Strategy::DEFAULT);

    void permuteViews();

private:
    Ui::GalleryView *ui;

    QGridLayout* _gridLayout;

    std::vector<std::shared_ptr<IContentView>> _views;
};

#endif // GALLERY_VIEW_H
