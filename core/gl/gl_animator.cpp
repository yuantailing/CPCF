#include "gl_animator.h"

void gl::aniArcball::OnUserInputEvent(const os::UserInputEvent& e)
{
	if(e.Type == os::UIEVT_MOUSE_DRAGBEGIN && e.ButtonCode == os::BUTTON_MOUSE_LEFT)
	{	
		_arcball.OnDragBegin((rt::Vec2i&)e.Position);
		UpdateStateVersion();
	}
	else if(e.Type == os::UIEVT_MOUSE_DRAG && e.ButtonCode == os::BUTTON_MOUSE_LEFT)
	{	_arcball.OnDrag((rt::Vec2i&)e.Position);
		UpdateStateVersion();
	}
	else if(e.Type == os::UIEVT_VIEWPORT_RESIZE)
	{
		_arcball.SetViewport(0,0,e.Size.width-1, e.Size.height-1);
		UpdateStateVersion();
	}
}

const rt::Mat4x4f& gl::aniArcball::GetTransformation() const
{
	return _arcball.GetMatrix();
}

