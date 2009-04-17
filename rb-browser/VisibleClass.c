/*
 * VisibleClass.c
 */

#include "MHEGEngine.h"
#include "ISO13522-MHEG-5.h"
#include "VideoClass.h"
#include "RTGraphicsClass.h"
#include "StreamClass.h"
#include "BitmapClass.h"
#include "LineArtClass.h"
#include "DynamicLineArtClass.h"
#include "RectangleClass.h"
#include "HotspotClass.h"
#include "SwitchButtonClass.h"
#include "PushButtonClass.h"
#include "TextClass.h"
#include "EntryFieldClass.h"
#include "HyperTextClass.h"
#include "SliderClass.h"
#include "rtti.h"

void
VisibleClass_Activation(RootClass *v)
{
	switch(v->inst.rtti)
	{
	case RTTI_VideoClass:
		VideoClass_Activation((VideoClass *) v);
		break;

	case RTTI_RTGraphicsClass:
		RTGraphicsClass_Activation((RTGraphicsClass *) v);
		break;

	case RTTI_BitmapClass:
		BitmapClass_Activation((BitmapClass *) v);
		break;

	case RTTI_LineArtClass:
		LineArtClass_Activation((LineArtClass *) v);
		break;

	case RTTI_DynamicLineArtClass:
		DynamicLineArtClass_Activation((DynamicLineArtClass *) v);
		break;

	case RTTI_RectangleClass:
		RectangleClass_Activation((RectangleClass *) v);
		break;

	case RTTI_HotspotClass:
		HotspotClass_Activation((HotspotClass *) v);
		break;

	case RTTI_SwitchButtonClass:
		SwitchButtonClass_Activation((SwitchButtonClass *) v);
		break;

	case RTTI_PushButtonClass:
		PushButtonClass_Activation((PushButtonClass *) v);
		break;

	case RTTI_TextClass:
		TextClass_Activation((TextClass *) v);
		break;

	case RTTI_EntryFieldClass:
		EntryFieldClass_Activation((EntryFieldClass *) v);
		break;

	case RTTI_HyperTextClass:
		HyperTextClass_Activation((HyperTextClass *) v);
		break;

	case RTTI_SliderClass:
		SliderClass_Activation((SliderClass *) v);
		break;

	default:
		error("Unknown VisibleClass type: %u", v->inst.rtti);
		break;
	}

	return;
}

/*
 * these are the only Visible classes we need to support in the UK MHEG Profile
 */

void
VisibleClass_render(RootClass *v, MHEGDisplay *d, XYPosition *pos, OriginalBoxSize *box)
{
	switch(v->inst.rtti)
	{
	case RTTI_VideoClass:
		VideoClass_render((VideoClass *) v, d, pos, box);
		break;

	case RTTI_BitmapClass:
		BitmapClass_render((BitmapClass *) v, d, pos, box);
		break;

	case RTTI_DynamicLineArtClass:
		DynamicLineArtClass_render((DynamicLineArtClass *) v, d, pos, box);
		break;

	case RTTI_RectangleClass:
		RectangleClass_render((RectangleClass *) v, d, pos, box);
		break;

	case RTTI_TextClass:
		TextClass_render((TextClass *) v, d, pos, box);
		break;

	case RTTI_EntryFieldClass:
		EntryFieldClass_render((EntryFieldClass *) v, d, pos, box);
		break;

	case RTTI_HyperTextClass:
		HyperTextClass_render((HyperTextClass *) v, d, pos, box);
		break;

	case RTTI_SliderClass:
		SliderClass_render((SliderClass *) v, d, pos, box);
		break;

	default:
		error("Unknown VisibleClass type: %u", v->inst.rtti);
		break;
	}

	return;
}

