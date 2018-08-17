#pragma once

#include "../../../essentials.h"
#include "wnd_app.h"
#include "../tests/render_core.h"


class MainWnd: public os::WndBase
{
protected:
	RenderCore		_core;
	bool			OnInitWnd() override;
};