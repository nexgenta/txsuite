/* -*- c -*- ****************************************************************
 *
 *               Copyright 2003 Samsung Electronics (UK) Ltd.
 *               Copyright 2005,2007 SpongeLava Ltd.
 * 
 *
 *  System        : mhegenc
 *  Module        : mhegenc
 *
 *  Object Name   : $RCSfile$
 *  Revision      : $Revision: 1873 $
 *  Date          : $Date: 2008-08-24 14:36:54 +0100 (Sun, 24 Aug 2008) $
 *  Author        : $Author: pthomas $
 *  Created By    : Paul Thomas
 *  Create Date   : Mon Jun 9 2003
 *
 *  Description 
 *               bison definitions for mhegenc.
 *  Notes
 *
 *  History
 * 
 *   0.9   Oct 2007 Added SetDesktopColour and OrigDesktopColour for
 *                  freesat extensions.
 *      
 ****************************************************************************
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 ****************************************************************************/
%{
#include <string.h>
#include "asn-incl.h"
#include "mh_snacc.h"
#include "lexer.h"
#include "parser.h"
#include "diag.h"
#define YYERROR_VERBOSE



/* Globals. */
int testing_mode;
InterchangedObject snacc_object;
#define snaccAlloc(s) NibbleAlloc(s)

/* Locals. */
#define AsnFalse 0
#define AsnTrue (~AsnFalse)

static const char *rcsid =
    "@(#) $Id: mheg.y 1873 2008-08-24 13:36:54Z pthomas $";

static AsnOcts current_group_id;
static int found_app;
static int found_scene;

static void
yyerror(char *mess);
static AsnOcts
decode_string(AsnOcts in);
static void
warn_default(const char *field, int line);
static void
warn_bad_field(const char *class, const char *field, int line);
#define warn(message, line)     diag_msg(eSEVERITY_WARNING, message, line)
#define warn_strict(message, line) diag_msg(eSEVERITY_WARNING, message, line)
#define warn_restricted_optional(field, line) \
        diag_msg(eSEVERITY_WARNING, "OPTIONAL " field, line)
static void add_object_number(AsnInt,int);

%}

%union {
    AsnInt ival;
    AsnInt *ip;
    AsnOcts sval;
    AsnOcts *sp;
    AsnBool bval;
    AsnBool *bp;
    ObjectReference *object_reference;
    ActionClass *action;
    AsnList *listp;
    struct AspectRatio* aspect_ratio;
    GroupItem *group_item;

    struct LinkClass* link;
    ResidentProgramClass* resident_program;
    struct RemoteProgramClass* remote_program;
    InterchangedProgramClass* interchanged_program;
    PaletteClass* palette;
    FontClass* font_class;
    CursorShapeClass* cursor_shape;
    BooleanVariableClass* boolean_variable;
    IntegerVariableClass* integer_variable;
    OctetStringVariableClass* octet_string_variable;
    ObjectRefVariableClass* object_ref_variable;
    ContentRefVariableClass* content_ref_variable;
    struct TokenGroupClass* token_group;
    struct ListGroupClass* list_group;
    struct StreamClass* stream;
    struct BitmapClass* bitmap;
    struct LineArtClass* line_art;
    DynamicLineArtClass* dynamic_line_art;
    RectangleClass* rectangle;
    HotspotClass* hotspot;
    struct SwitchButtonClass* switch_button;
    struct PushButtonClass* push_button;
    struct TextClass* text;
    struct EntryFieldClass* entry_field;
    struct HyperTextClass* hyper_text;
    struct SliderClass* slider;

    AudioClass *audio;
    VideoClass *video;
    RTGraphicsClass *rtg;

    DefaultAttribute* default_attribute;
    struct Colour* colour;
    struct FontBody* font;
    NextScene* next_scene;

    struct ContentBody* content_body;
    EventData *event_data;

    struct OriginalValue* original_value;

    ActionSlot *action_slot;

    TokenGroupItem *token_group_item;

    XYPosition *position;

    StreamComponent* stream_component;

    ElementaryAction* e_action;
    NewContent* new_content;
    NewColour *new_colour;
    NewFont * new_font;
    Rational *rational;
    NewVariableValue *new_var_val;
    ComparisonValue *comparison_val;
    Parameter *param;
    Point *point;

    NewContentSize *new_content_size;

    GenericBoolean *gbool;
    GenericInteger *gint;
    GenericOctetString *gos;
    GenericObjectReference *gobj;
    GenericContentReference *gcont;
    EmulatedEventData *emulated_event_data;
}

/* Root */
%type <object_reference> ObjectReference

/* Group */
%type <ival> StandardIdentifier   /* not actualy an int, but expect 0 */
%type <ival> StandardVersion
%type <sval> ObjectInformation
%type <action> OnStartUp
%type <action> OnCloseDown
%type <ival> OrigGroupCPriority
%type <listp> Items
%type <listp> GroupItemList

%type <group_item> GroupItem

/* Application. */
%type <object_reference> ApplicationId

%type <action> OnSpawnCloseDown
%type <action> OnRestart
%type <listp> DefaultAttributes
%type <listp> DefaultAttributeList
%type <default_attribute> DefaultAttribute

%type <colour> Colour
%type <font> FontBody

%type <colour> OriginalDesktopColour

/* Scene */
%type <object_reference> SceneId

%type <aspect_ratio> AspectRatio
%type <bval> MovingCursor
%type <listp> NextScenes
%type <listp> NextSceneList
%type <next_scene> NextScene

/* Link */
%type <link> LinkClass

%type <bval> InitiallyActive
%type <bval> InvInitiallyActive
%type <ip> ContentHook
%type <content_body> OriginalContent
%type <bval> Shared

%type <ip> ContentSize
%type <ip> ContentCachePriority

%type <event_data> EventData
%type <bval> InitiallyAvailable
%type <resident_program> ResidentProgramClass

%type <remote_program> RemoteProgramClass
%type <interchanged_program> InterchangedProgramClass
%type <palette> PaletteClass
%type <font_class> FontClass
%type <cursor_shape> CursorShapeClass

%type <original_value> OrigValue
%type <boolean_variable> BooleanVariableClass
%type <integer_variable> IntegerVariableClass
%type <octet_string_variable> OctetStringVariableClass
%type <object_ref_variable> ObjectRefVariableClass
%type <content_ref_variable> ContentRefVariableClass

%type <listp> TokenManager
%type <listp> MovementList
%type <listp> Movement
%type <listp> IntegerList

%type <token_group> TokenGroupClass
%type <listp> TokenGroupItems
%type <listp> TokenGroupItemList
%type <listp> NoTokenActionSlots

%type <token_group_item> TokenGroupItem
%type <listp> ActionSlots

%type <listp> ActionSlotList
%type <action_slot> ActionSlot


%type <list_group> ListGroupClass
%type <listp> PositionList
%type <position> Position
%type <bval> WrapAround
%type <bval> MultipleSelection

%type <position> OriginalPosition
%type <object_reference> OriginalPaletteRef

%type <bitmap> BitmapClass
%type <bval> Tiling
%type <ival> OriginalTransparency

%type <line_art> LineArtClass
%type <line_art> LineArtBody
%type <bval> BorderedBoundingBox
%type <ival> OriginalLineWidth
%type <ival> OriginalLineStyle
%type <colour> OriginalRefLineColour
%type <colour> OriginalRefFillColour

%type <rectangle> RectangleClass
%type <dynamic_line_art> DynamicLineArtClass

%type <text> TextClass
%type <font> OriginalFont
%type <sval> OptionalFontAttributes
%type <colour> OptionalTextColour
%type <colour> OptionalBackgroundColour
%type <ip> OptionalCharacterSet
%type <ival> HorizontalJustification
%type <ival> VerticalJustification
%type <ival> JustificationEnum
%type <ival> LineOrientation
%type <ival> LineOrientationEnum
%type <ival> StartCorner
%type <ival> StartCornerEnum
%type <bval> TextWrapping

%type <stream> StreamClass
%type <listp> Multiplex
%type <listp> StreamComponentList
%type <stream_component> StreamComponent
%type <ival> Storage
%type <ival> StorageEnum
%type <ival> Looping

%type <audio> AudioClass
%type <ival> OriginalVolume

%type <video> VideoClass
%type <ival> Termination
%type <ival> TerminationEnum

%type <rtg> RTGraphicsClass

%type <bval> EngineResp
%type <colour> HighlightRefColour

%type <slider> SliderClass
%type <ival> MinValue
%type <ip> InitialValue
%type <ip> InitialPortion
%type <ival> StepSize
%type <ival> OrientationEnum
%type <ival> SliderStyle
%type <ival> SliderStyleEnum
%type <colour> SliderRefColour

%type <entry_field> EntryFieldClass
%type <ival> InputType
%type <ival> InputTypeEnum
%type <sval> CharList
%type <bval> ObscuredInput
%type <ival> MaxLength

%type <hyper_text> HyperTextClass

%type <colour> ButtonRefColour

%type <hotspot> HotspotClass

%type <push_button> PushButtonClass

%type <sval> OriginalLabel

%type <switch_button> SwitchButtonClass
%type <ival> ButtonStyleEnum


%type <listp> ElementaryActionList
%type <e_action> ElementaryAction

%type <listp> Parameters
%type <listp> ParameterList
%type <param> Parameter

%type <listp> PointList
%type <point> Point

%type <listp> VariableList

%type <emulated_event_data> EmulatedEventData
%type <ival> EventTypeEnum

%type <gint> NewCounterValue
%type <gobj> NewCursorShape
%type <new_content> NewContent
%type <new_colour> NewColour
%type <new_colour> NewColourOrNone
%type <new_font> NewFont
%type <rational> Rational
%type <gint> TimerValue
%type <gbool> AbsoluteTime
%type <new_var_val> NewVariableValue
%type <comparison_val> ComparisonValue
%type <gint> ComparisonOperator
%type <gint> ConnectionTag
%type <gint> TransitionEffect

%type <new_content_size> NewContentSize
%type <gint> NewContentCachePriority

%type <gobj> GObjRef
%type <gcont> GContRef
%type <gint> GInt
%type <gbool> GBool
%type <gos> GOString

%type <action> ActionClass


%type <ival> Integer


%token <ival> eTOK_INTEGER
%token <ival> eTOK_BOOLEAN
%token <sval> eTOK_STRING
%token eTOK_NULL
%token <sval>eTOK_ENUM
%token eTOK_LBRACE
%token eTOK_RBRACE
%token eTOK_LPAREN
%token eTOK_RPAREN

%token eTOK_TAG_StandardVersion
%token eTOK_TAG_StandardIdentifier
%token eTOK_TAG_ObjectInformation
%token eTOK_TAG_OnStartUp
%token eTOK_TAG_OnCloseDown
%token eTOK_TAG_Items
%token eTOK_TAG_OriginalGroupCachePriority
%token eTOK_TAG_OnSpawnCloseDown
%token eTOK_TAG_OnRestart
%token eTOK_TAG_CharacterSet
%token eTOK_TAG_BackgroundColour
%token eTOK_TAG_TextContentHook
%token eTOK_TAG_TextColour
%token eTOK_TAG_FontAttributes
%token eTOK_TAG_Font
%token eTOK_TAG_IProgramContentHook
%token eTOK_TAG_StreamContentHook
%token eTOK_TAG_BitmapContentHook
%token eTOK_TAG_LineArtContentHook
%token eTOK_TAG_ButtonRefColour
%token eTOK_TAG_HighlightRefColour
%token eTOK_TAG_SliderRefColour
%token eTOK_TAG_OriginalDesktopColour
%token eTOK_TAG_InputEventRegister
%token eTOK_TAG_SceneCoordinateSystem
%token eTOK_TAG_AspectRatio
%token eTOK_TAG_MovingCursor
%token eTOK_TAG_NextScenes
%token eTOK_TAG_InitiallyActive
%token eTOK_TAG_ContentHook
%token eTOK_TAG_OriginalContent
%token eTOK_TAG_ContentRef
%token eTOK_TAG_ContentSize
%token eTOK_TAG_ContentCachePriority
%token eTOK_TAG_Shared
%token eTOK_TAG_EventSource
%token eTOK_TAG_EventType
%token eTOK_TAG_EventData
%token eTOK_TAG_LinkEffect
%token eTOK_TAG_Activate
%token eTOK_TAG_Add
%token eTOK_TAG_AddItem 
%token eTOK_TAG_Append 
%token eTOK_TAG_BringToFront 
%token eTOK_TAG_Call
%token eTOK_TAG_CallActionSlot 
%token eTOK_TAG_Clear 
%token eTOK_TAG_Clone 
%token eTOK_TAG_CloseConnection 
%token eTOK_TAG_Deactivate 
%token eTOK_TAG_DelItem 
%token eTOK_TAG_Deselect 
%token eTOK_TAG_DeselectItem 
%token eTOK_TAG_Divide 
%token eTOK_TAG_DrawArc 
%token eTOK_TAG_DrawLine 
%token eTOK_TAG_DrawOval 
%token eTOK_TAG_DrawPolygon 
%token eTOK_TAG_DrawPolyline 
%token eTOK_TAG_DrawRectangle 
%token eTOK_TAG_DrawSector 
%token eTOK_TAG_Fork 
%token eTOK_TAG_GetAvailabilityStatus 
%token eTOK_TAG_GetBoxSize 
%token eTOK_TAG_GetCellItem 
%token eTOK_TAG_GetCursorPosition 
%token eTOK_TAG_GetEngineSupport 
%token eTOK_TAG_GetEntryPoint 
%token eTOK_TAG_GetFillColour 
%token eTOK_TAG_GetFirstItem 
%token eTOK_TAG_GetHighlightStatus 
%token eTOK_TAG_GetInteractionStatus 
%token eTOK_TAG_GetItemStatus 
%token eTOK_TAG_GetLabel 
%token eTOK_TAG_GetLastAnchorFired 
%token eTOK_TAG_GetLineColour 
%token eTOK_TAG_GetLineStyle 
%token eTOK_TAG_GetLineWidth 
%token eTOK_TAG_GetListItem 
%token eTOK_TAG_GetListSize 
%token eTOK_TAG_GetOverwriteMode 
%token eTOK_TAG_GetPortion 
%token eTOK_TAG_GetPosition 
%token eTOK_TAG_GetRunningStatus 
%token eTOK_TAG_GetSelectionStatus 
%token eTOK_TAG_GetSliderValue 
%token eTOK_TAG_GetTextContent 
%token eTOK_TAG_GetTextData
%token eTOK_TAG_GetTokenPosition 
%token eTOK_TAG_GetVolume 
%token eTOK_TAG_Launch 
%token eTOK_TAG_LockScreen 
%token eTOK_TAG_Modulo 
%token eTOK_TAG_Move 
%token eTOK_TAG_MoveTo 
%token eTOK_TAG_Multiply 
%token eTOK_TAG_OpenConnection 
%token eTOK_TAG_Preload 
%token eTOK_TAG_PutBefore 
%token eTOK_TAG_PutBehind 
%token eTOK_TAG_Quit 
%token eTOK_TAG_ReadPersistent 
%token eTOK_TAG_Run 
%token eTOK_TAG_ScaleBitmap 
%token eTOK_TAG_ScaleVideo 
%token eTOK_TAG_ScrollItems 
%token eTOK_TAG_Select 
%token eTOK_TAG_SelectItem 
%token eTOK_TAG_SendEvent 
%token eTOK_TAG_SendToBack 
%token eTOK_TAG_SetBoxSize 
%token eTOK_TAG_SetCachePriority 
%token eTOK_TAG_SetCounterEndPosition 
%token eTOK_TAG_SetCounterPosition 
%token eTOK_TAG_SetCounterTrigger 
%token eTOK_TAG_SetCursorPosition 
%token eTOK_TAG_SetCursorShape 
%token eTOK_TAG_SetData 
%token eTOK_TAG_SetEntryPoint 
%token eTOK_TAG_SetFillColour 
%token eTOK_TAG_SetFirstItem 
%token eTOK_TAG_SetFontRef 
%token eTOK_TAG_SetHighlightStatus 
%token eTOK_TAG_SetInteractionStatus 
%token eTOK_TAG_SetLabel 
%token eTOK_TAG_SetLineColour 
%token eTOK_TAG_SetLineStyle 
%token eTOK_TAG_SetLineWidth 
%token eTOK_TAG_SetOverwriteMode 
%token eTOK_TAG_SetPaletteRef 
%token eTOK_TAG_SetPortion
%token eTOK_TAG_SetPosition 
%token eTOK_TAG_SetSliderValue 
%token eTOK_TAG_SetSpeed 
%token eTOK_TAG_SetTimer 
%token eTOK_TAG_SetTransparency 
%token eTOK_TAG_SetVariable 
%token eTOK_TAG_SetVolume 
%token eTOK_TAG_Spawn 
%token eTOK_TAG_Step 
%token eTOK_TAG_Stop 
%token eTOK_TAG_StorePersistent 
%token eTOK_TAG_Subtract 
%token eTOK_TAG_TestVariable 
%token eTOK_TAG_Toggle 
%token eTOK_TAG_ToggleItem 
%token eTOK_TAG_TransitionTo 
%token eTOK_TAG_Unload 
%token eTOK_TAG_UnlockScreen
%token eTOK_TAG_SetBackgroundColour
%token eTOK_TAG_SetTextColour
%token eTOK_TAG_SetFontAttributes
%token eTOK_TAG_SetCellPosition
%token eTOK_TAG_SetInputRegister

%token eTOK_TAG_SetBitmapDecodeOffset
%token eTOK_TAG_GetBitmapDecodeOffset
%token eTOK_TAG_SetVideoDecodeOffset
%token eTOK_TAG_GetVideoDecodeOffset

%token eTOK_TAG_SetFocusPosition
%token eTOK_TAG_GetFocusPosition
%token eTOK_TAG_SetSliderParameters

%token eTOK_TAG_SetDesktopColour

%token eTOK_TAG_IndirectReference
%token eTOK_TAG_GenericBoolean
%token eTOK_TAG_GenericInteger
%token eTOK_TAG_GenericOctetString
%token eTOK_TAG_GenericObjectReference
%token eTOK_TAG_GenericContentReference
%token eTOK_TAG_ConnectionTag
%token eTOK_TAG_Name
%token eTOK_TAG_NewContentSize
%token eTOK_TAG_NewContentCachePriority
%token eTOK_TAG_NewReferencedContent
%token eTOK_TAG_NewColourIndex
%token eTOK_TAG_NewAbsoluteColour
%token eTOK_TAG_AbsoluteTime
%token eTOK_TAG_InitiallyAvailable
%token eTOK_TAG_OriginalValue
%token eTOK_TAG_ObjectReferenceValue
%token eTOK_TAG_MovementTable
%token eTOK_TAG_TokenGroupItems
%token eTOK_TAG_NoTokenActionSlots
%token eTOK_TAG_Positions
%token eTOK_TAG_ActionSlots
%token eTOK_TAG_WrapAround
%token eTOK_TAG_MultipleSelection
%token eTOK_TAG_OriginalBoxSize
%token eTOK_TAG_OriginalPosition
%token eTOK_TAG_OriginalPaletteRef
%token eTOK_TAG_Tiling
%token eTOK_TAG_OriginalTransparency
%token eTOK_TAG_BorderedBoundingBox
%token eTOK_TAG_OriginalLineWidth
%token eTOK_TAG_OriginalLineStyle
%token eTOK_TAG_OriginalRefFillColour
%token eTOK_TAG_OriginalRefLineColour
%token eTOK_TAG_OriginalFont
%token eTOK_TAG_HorizontalJustification
%token eTOK_TAG_VerticalJustification
%token eTOK_TAG_LineOrientation
%token eTOK_TAG_StartCorner
%token eTOK_TAG_TextWrapping
%token eTOK_TAG_Multiplex
%token eTOK_TAG_Storage
%token eTOK_TAG_Looping
%token eTOK_TAG_ComponentTag
%token eTOK_TAG_OriginalVolume
%token eTOK_TAG_Termination
%token eTOK_TAG_EngineResp
%token eTOK_TAG_Orientation
%token eTOK_TAG_MaxValue
%token eTOK_TAG_MinValue
%token eTOK_TAG_InitialValue
%token eTOK_TAG_InitialPortion
%token eTOK_TAG_InitialPosition
%token eTOK_TAG_StepSize
%token eTOK_TAG_SliderStyle
%token eTOK_TAG_InputType
%token eTOK_TAG_CharList
%token eTOK_TAG_ObscuredInput
%token eTOK_TAG_MaxLength
%token eTOK_TAG_OriginalLabel
%token eTOK_TAG_ButtonStyle

%token eTOK_TAG_Application
%token eTOK_TAG_Scene
%token eTOK_TAG_Link
%token eTOK_TAG_ResidentProgram
%token eTOK_TAG_RemoteProgram
%token eTOK_TAG_InterchangedProgram
%token eTOK_TAG_Palette
 /* %token eTOK_TAG_Font */
%token eTOK_TAG_CursorShape
%token eTOK_TAG_BooleanVariable
%token eTOK_TAG_IntegerVariable
%token eTOK_TAG_OctetStringVariable
%token eTOK_TAG_ObjectRefVariable
%token eTOK_TAG_ContentRefVariable
%token eTOK_TAG_TokenGroup
%token eTOK_TAG_ListGroup
%token eTOK_TAG_Bitmap
%token eTOK_TAG_LineArt
%token eTOK_TAG_Rectangle
%token eTOK_TAG_DynamicLineArt
%token eTOK_TAG_Text
%token eTOK_TAG_Stream
%token eTOK_TAG_Audio
%token eTOK_TAG_Video
%token eTOK_TAG_RTGraphics
%token eTOK_TAG_Slider
%token eTOK_TAG_EntryField
%token eTOK_TAG_HyperText
%token eTOK_TAG_Hotspot
%token eTOK_TAG_PushButton
%token eTOK_TAG_SwitchButton

%token eTOK_ENUM_IsAvailable
%token eTOK_ENUM_ContentAvailable
%token eTOK_ENUM_IsDeleted
%token eTOK_ENUM_IsRunning
%token eTOK_ENUM_IsStopped
%token eTOK_ENUM_UserInput
%token eTOK_ENUM_AnchorFired
%token eTOK_ENUM_TimerFired
%token eTOK_ENUM_AsynchStopped
%token eTOK_ENUM_InteractionCompleted
%token eTOK_ENUM_TokenMovedFrom
%token eTOK_ENUM_TokenMovedTo
%token eTOK_ENUM_StreamEvent
%token eTOK_ENUM_StreamPlaying
%token eTOK_ENUM_StreamStopped
%token eTOK_ENUM_CounterTrigger
%token eTOK_ENUM_HighlightOn
%token eTOK_ENUM_HighlightOff
%token eTOK_ENUM_CursorEnter
%token eTOK_ENUM_CursorLeave
%token eTOK_ENUM_IsSelected
%token eTOK_ENUM_IsDeselected
%token eTOK_ENUM_TestEvent
%token eTOK_ENUM_FirstItemPresented
%token eTOK_ENUM_LastItemPresented
%token eTOK_ENUM_HeadItems
%token eTOK_ENUM_TailItems
%token eTOK_ENUM_ItemSelected
%token eTOK_ENUM_ItemDeselected
%token eTOK_ENUM_EntryFieldFull
%token eTOK_ENUM_EngineEvent
%token eTOK_ENUM_FocusMoved
%token eTOK_ENUM_SliderValueChanged
%token eTOK_ENUM_start
%token eTOK_ENUM_end
%token eTOK_ENUM_centre
%token eTOK_ENUM_justified
%token eTOK_ENUM_vertical
%token eTOK_ENUM_horizontal
%token eTOK_ENUM_upper_left
%token eTOK_ENUM_upper_right
%token eTOK_ENUM_lower_left
%token eTOK_ENUM_lower_right
%token eTOK_ENUM_memory
%token eTOK_ENUM_stream
%token eTOK_ENUM_freeze
%token eTOK_ENUM_disappear
%token eTOK_ENUM_left
%token eTOK_ENUM_right
%token eTOK_ENUM_up
%token eTOK_ENUM_down  
%token eTOK_ENUM_normal
%token eTOK_ENUM_thermometer
%token eTOK_ENUM_proportional
%token eTOK_ENUM_alpha
%token eTOK_ENUM_numeric
%token eTOK_ENUM_any
%token eTOK_ENUM_listed 
%token eTOK_ENUM_pushbutton
%token eTOK_ENUM_radiobutton
%token eTOK_ENUM_checkbox

%token eTOK_EXT_Equals
%token eTOK_EXT_NotEquals
%token eTOK_EXT_StrictlyLess
%token eTOK_EXT_LessOrEqual
%token eTOK_EXT_StrictlyGreater
%token eTOK_EXT_GreaterOrEqual
%token eTOK_EXT_Plus
%token eTOK_EXT_Minus
%token eTOK_EXT_Multiply
%token eTOK_EXT_Divide

%%

/* Start Symbol */
/* Constructs a snacc InterchangedObject and save it in the snacc_object
 * global
 */
InterchangedObject: eTOK_LBRACE ApplicationId
                                StandardIdentifier
                                StandardVersion 
                                ObjectInformation 
                                OnStartUp 
                                OnCloseDown 
                                OrigGroupCPriority 
                                Items 
                                OnSpawnCloseDown 
                                OnRestart 
                                DefaultAttributes
                                OriginalDesktopColour
                    eTOK_RBRACE
{
    snacc_object.choiceId = INTERCHANGEDOBJECT_APPLICATION;
    snacc_object.a.application = snaccAlloc(sizeof(ApplicationClass));

    snacc_object.a.application->rootClass = $2;
    
    /* ObjectInformation */
    snacc_object.a.application->object_information = $5;
    
    /* OnStartUp */
    snacc_object.a.application->on_start_up = $6;
    /* OnCloseDown */
    snacc_object.a.application->on_close_down = $7;
    /* OrigGroupCPriority */
    if( $8 != 127 ) {
        snacc_object.a.application->original_group_cache_priority
            = snaccAlloc(sizeof(AsnInt));
        *(snacc_object.a.application->original_group_cache_priority) = $8;
    }
    /* Items */
    snacc_object.a.application->items = $9;
    /* OnSpawnCloseDown */
    snacc_object.a.application->on_spawn_close_down = $10;
    /* OnRestart */
    snacc_object.a.application->on_restart = $11;
    /* DefaultAttributes */
    snacc_object.a.application->default_attributes = $12;
    /* OriginalDesktopColour */
    snacc_object.a.application->original_desktop_colour = $13;
    if( diag_count( eSEVERITY_ERROR ) )
    {
        diag_report( "*** Unable to encode Application");
        YYABORT;
    }
    else
        diag_report( "Encoded Application" );

}
| eTOK_LBRACE SceneId
              StandardIdentifier
              StandardVersion 
              ObjectInformation 
              OnStartUp 
              OnCloseDown 
              OrigGroupCPriority 
              Items 
              eTOK_TAG_InputEventRegister Integer
              eTOK_TAG_SceneCoordinateSystem Integer Integer
              AspectRatio
              MovingCursor
              NextScenes
   eTOK_RBRACE
{
    snacc_object.choiceId = INTERCHANGEDOBJECT_SCENE;
    snacc_object.a.scene = snaccAlloc(sizeof(SceneClass));
    snacc_object.a.scene->rootClass = $2;
    
    /* ObjectInformation */
    snacc_object.a.scene->object_information = $5;
    
    /* OnStartUp */
    snacc_object.a.scene->on_start_up = $6;
    /* OnCloseDown */
    snacc_object.a.scene->on_close_down = $7;
    /* OrigGroupCPriority */
    if( $8 != 127 ){
        snacc_object.a.scene->original_group_cache_priority
            = snaccAlloc(sizeof(AsnInt));
        *(snacc_object.a.scene->original_group_cache_priority) = $8;
    }
    /* Items */
    snacc_object.a.scene->items = $9;
    
    /* InputEventRegister */
    snacc_object.a.scene->input_event_register = $11;
    
    /* SceneCoordinateSystem */
    snacc_object.a.scene->scene_coordinate_system
        = snaccAlloc(sizeof(struct SceneCoordinateSystem));
    snacc_object.a.scene->scene_coordinate_system->x_scene = $13;
    snacc_object.a.scene->scene_coordinate_system->y_scene = $14;
    
    /* AspectRatio */
    snacc_object.a.scene->aspect_ratio = $15;
    /* MovingCursor */
    if( $16 ) {
        /* not default. */
        snacc_object.a.scene->moving_cursor = snaccAlloc(sizeof(AsnBool));
        *(snacc_object.a.scene->moving_cursor) = AsnTrue;
    }
    /* NextScenes */
    snacc_object.a.scene->next_scenes = $17;
    if( diag_count( eSEVERITY_ERROR ) )
    {
        diag_report( "*** Unable to encode Scene");
        YYABORT;
    }
    else
        diag_report( "Encoded Scene" );
}
| error
{
    if( found_app )
        diag_report( "*** Unable to encode Application" );
    else if( found_scene )
        diag_report( "*** Unable to encode Scene");
    else
        diag_report( "*** No Application or Scene found");
    YYABORT;
}
;

ApplicationId: eTOK_TAG_Application ObjectReference
{
    $$ = snaccAlloc(sizeof(*$$));
    /* ObjectReference */
    $$ = $2;
    if( $2->choiceId == OBJECTREFERENCE_INTERNAL_REFERENCE )
        diag_msg(eSEVERITY_WARNING,
                 "Application must have external reference", @2.first_line);
    else {
        if( $2->a.external_reference->object_number )
            diag_msg(eSEVERITY_WARNING,
                     "Application must have 0 object number", @2.first_line);
        current_group_id = $2->a.external_reference->group_identifier;
    }
    found_app = 1;
}
;

SceneId: eTOK_TAG_Scene ObjectReference
{
    $$ = snaccAlloc(sizeof(*$$));
    /* ObjectReference */
    $$ = $2;
    if( $2->choiceId == OBJECTREFERENCE_INTERNAL_REFERENCE )
        diag_msg(eSEVERITY_WARNING,
                 "Scene must have external reference", @2.first_line);
    else {
        if( $2->a.external_reference->object_number )
            diag_msg(eSEVERITY_WARNING,
                     "Scene must have 0 object number", @2.first_line);
        current_group_id = $2->a.external_reference->group_identifier;
    }
    found_scene = 1;
}
;

OnSpawnCloseDown: eTOK_TAG_OnSpawnCloseDown ActionClass {$$=$2;}
| /* empty */  {$$=0;}
;
OnRestart: eTOK_TAG_OnRestart ActionClass {$$=$2;}
| /* empty */  {$$=0;}
;
DefaultAttributes: DefaultAttributeList {$$=$1}
| /* empty */ {$$=0;}
;
DefaultAttributeList: DefaultAttribute DefaultAttributeList
{
    /* The prepend will create a node and return pointer to the data.
     * a DefaultAttribute is a two word structure, so just assign it.
     */
    *((DefaultAttribute **)AsnListPrepend($2)) = $1;
    $$ = $2;
}
|DefaultAttribute
{
    /* This should be the last in the list (first reduce) so create the AsnList
     * and the other alternate can prepend to it.
     */
    $$ = AsnListNew(sizeof($1));
    /* The prepend will create a node and return pointer to the data.
     * a DefaultAttribute is a two word structure, so just assign it.
     */
    *((DefaultAttribute **)AsnListPrepend($$)) = $1;
}
;
DefaultAttribute: eTOK_TAG_CharacterSet Integer
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = DEFAULTATTRIBUTE_CHARACTER_SET;
    $$->a.character_set = $2;
}
| eTOK_TAG_BackgroundColour Colour
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = DEFAULTATTRIBUTE_BACKGROUND_COLOUR;
    $$->a.background_colour = $2;
}
| eTOK_TAG_TextContentHook Integer
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = DEFAULTATTRIBUTE_TEXT_CONTENT_HOOK;
    $$->a.text_content_hook = $2;
}
| eTOK_TAG_TextColour Colour
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = DEFAULTATTRIBUTE_TEXT_COLOUR;
    $$->a.text_colour = $2;
}
| eTOK_TAG_Font FontBody
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = DEFAULTATTRIBUTE_FONT;
    $$->a.font = $2;
}
| eTOK_TAG_FontAttributes eTOK_STRING
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = DEFAULTATTRIBUTE_FONT_ATTRIBUTES;
    $$->a.font_attributes = snaccAlloc(sizeof($2));
    *($$->a.font_attributes) = decode_string($2);
}
| eTOK_TAG_IProgramContentHook Integer 
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = DEFAULTATTRIBUTE_INTERCHANGED_PROGRAM_CONTENT_HOOK;
    $$->a.interchanged_program_content_hook = $2;
}
| eTOK_TAG_StreamContentHook Integer
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = DEFAULTATTRIBUTE_STREAM_CONTENT_HOOK;
    $$->a.stream_content_hook = $2;
}
| eTOK_TAG_BitmapContentHook Integer
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = DEFAULTATTRIBUTE_BITMAP_CONTENT_HOOK;
    $$->a.bitmap_content_hook = $2;
}
| eTOK_TAG_LineArtContentHook Integer 
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = DEFAULTATTRIBUTE_LINE_ART_CONTENT_HOOK;
    $$->a.line_art_content_hook = $2;
}
| eTOK_TAG_ButtonRefColour Colour
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = DEFAULTATTRIBUTE_BUTTON_REF_COLOUR;
    $$->a.button_ref_colour = $2;
}
| eTOK_TAG_HighlightRefColour Colour 
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = DEFAULTATTRIBUTE_HIGHLIGHT_REF_COLOUR;
    $$->a.highlight_ref_colour = $2;
}
| eTOK_TAG_SliderRefColour Colour
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = DEFAULTATTRIBUTE_SLIDER_REF_COLOUR;
    $$->a.slider_ref_colour = $2;
}
;

OriginalDesktopColour: eTOK_TAG_OriginalDesktopColour Colour
{ $$ = $2; }
| /* empty */  {$$=0;}
;


/* FontBody returns a pointer to a snacc struct FontBody */
FontBody: eTOK_STRING
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = FONTBODY_DIRECT_FONT;
    $$->a.direct_font = snaccAlloc(sizeof(AsnOcts));
    *($$->a.direct_font) = decode_string($1);
}
| ObjectReference
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = FONTBODY_INDIRECT_FONT;
    $$->a.indirect_font = $1
}
;
/* Colour returns a pointer to a snacc struct Colour */
Colour: Integer
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = COLOUR_COLOUR_INDEX;
    $$->a.colour_index = $1;
    warn("Colour Index not supported in this profile.", @1.first_line);
}
| eTOK_STRING
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = COLOUR_ABSOLUTE_COLOUR;
    $$->a.absolute_colour = snaccAlloc(sizeof(AsnOcts));
    *($$->a.absolute_colour) = decode_string($1);
}
;

AspectRatio: eTOK_TAG_AspectRatio Integer Integer
{
    $$=snaccAlloc(sizeof(*$$));
    $$->width = $2;
    $$->height = $3;
}
| /* empty. */{$$=0;}
;
MovingCursor: eTOK_TAG_MovingCursor eTOK_BOOLEAN
{
    if( $2 )
        $$=AsnTrue;
    else {
        warn_default("MovingCursor", @2.first_line);
        $$=AsnFalse;
    }
}
| /* empty. */ {$$=AsnFalse;}
;
NextScenes: eTOK_TAG_NextScenes eTOK_LPAREN NextSceneList eTOK_RPAREN {$$=$3;}
| /* empty. */ {$$=0;}
;
NextSceneList: NextScene NextSceneList
{
    /* The prepend will create a node and return pointer to the data.
     * a NextScene is a three word structure, so just assign it.
     */
    *((NextScene **)AsnListPrepend($2)) = $1;
    $$ = $2;
}
| NextScene
{
    /* This should be the last in the list (first reduce) so create the AsnList
     * and the other alternate can prepend to it.
     */
    $$ = AsnListNew(sizeof($1));
    /* The prepend will create a node and return pointer to the data.
     * a NextScene is a three word structure, so just assign it.
     */
    *((NextScene **)AsnListPrepend($$)) = $1;
}
;
NextScene: eTOK_LPAREN eTOK_STRING Integer eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->scene_ref = decode_string($2);
    $$->scene_weight = $3;
    if( $3 < 0 || $3 > 255 )
        warn_range("SceneWeight", @3.first_line);
}
;

/* Group. */
StandardIdentifier: eTOK_TAG_StandardIdentifier Integer Integer
{
    warn_restricted_optional("StandardIdentifier", @1.first_line);
    if( $2 != joint_iso_itu_t || $3 != mheg )
        warn_strict("StandardIdentifier incorrect", @1.first_line);
    $$=1;                       /* just indicate that it exists. */
}
| /* empty. */ {$$=0;}
;
StandardVersion : eTOK_TAG_StandardVersion Integer
{
    warn_restricted_optional("StandardVersion", @1.first_line);
    if( $2 != version )
        warn_strict("StandardVersion incorrect", @1.first_line);
    $$=1;
}
| /* empty. */ {$$=0;}
;
ObjectInformation: eTOK_TAG_ObjectInformation eTOK_STRING
{
    $$=decode_string($2);
}
| /* empty. */
{
    $$.octetLen=0;
    $$.octs = 0;
}
;
OnStartUp: eTOK_TAG_OnStartUp ActionClass {$$=$2;}
| /* empty. */ {$$=0;}
;
OnCloseDown: eTOK_TAG_OnCloseDown ActionClass {$$=$2;}
|  /* empty. */ {$$=0;}
;
OrigGroupCPriority: eTOK_TAG_OriginalGroupCachePriority Integer
{
    $$=$2;
    if( $2 == 127 )
        warn_default("OrigGroupCachePriority", @2.first_line);
    if( $2 < 0 || $2 > 255 )
        warn_range("OrigGroupCachePriority", @2.first_line);

}
|  /* empty. */ {$$=127;}
;
Items: eTOK_TAG_Items eTOK_LPAREN GroupItemList eTOK_RPAREN {$$=$3;}
| /* empty. */ {$$=0;}
;

/* Returns a snacc AsnList list of Group Items
 */
GroupItemList: GroupItem GroupItemList
{
    if( $1 && $2 )
    {
        GroupItem **pnew;
        
        /* The prepend will create a node and return pointer to the data.
         * a GroupItem is a two word structure, so just assign it.
         */
        pnew = (GroupItem **)AsnListPrepend($2);
        *pnew = $1;
        
        add_object_number(
            $1->a.resident_program->rootClass->a.internal_reference,
            @1.first_line);
    }
    $$ = $2;
}
| GroupItem
{
    /* This should be the last in the list (first reduce) so create the
     * AsnList and the other alternate can prepend to it.
     */
    $$ = AsnListNew(sizeof($1));
    if( $1 )
    {
        GroupItem **pnew;
        /* The prepend will create a node and return pointer to the data.
         * a GroupItem is a two word structure, so just assign it.
         */
        pnew = (GroupItem **)AsnListPrepend($$);
        *pnew = $1;
        
        /* Also make a object lookup table so we can check object numbers. */
        add_object_number(
            $1->a.resident_program->rootClass->a.internal_reference,
            @1.first_line );
    }
}
;
GroupItem: LinkClass
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = GROUPITEM_LINK;
    $$->a.link = $1;
}
| ResidentProgramClass
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = GROUPITEM_RESIDENT_PROGRAM;
    $$->a.resident_program = $1;
}
| RemoteProgramClass
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = GROUPITEM_REMOTE_PROGRAM;
    $$->a.remote_program = $1;
}
| InterchangedProgramClass
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = GROUPITEM_INTERCHANGED_PROGRAM;
    $$->a.interchanged_program = $1;
}
| PaletteClass
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = GROUPITEM_PALETTE;
    $$->a.palette = $1;
}
| FontClass
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = GROUPITEM_FONT;
    $$->a.font_class = $1;
}
| CursorShapeClass
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = GROUPITEM_CURSOR_SHAPE;
    $$->a.cursor_shape = $1;
}
| BooleanVariableClass
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = GROUPITEM_BOOLEAN_VARIABLE;
    $$->a.boolean_variable = $1;
}
| IntegerVariableClass
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = GROUPITEM_INTEGER_VARIABLE;
    $$->a.integer_variable = $1;
}
| OctetStringVariableClass
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = GROUPITEM_OCTET_STRING_VARIABLE;
    $$->a.octet_string_variable = $1;
}
| ObjectRefVariableClass
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = GROUPITEM_OBJECT_REF_VARIABLE;
    $$->a.object_ref_variable = $1;
}
| ContentRefVariableClass
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = GROUPITEM_CONTENT_REF_VARIABLE;
    $$->a.content_ref_variable = $1;
}
| StreamClass
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = GROUPITEM_STREAM;
    $$->a.stream = $1;
}
| BitmapClass
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = GROUPITEM_BITMAP;
    $$->a.bitmap = $1;
}
| LineArtClass
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = GROUPITEM_LINE_ART;
    $$->a.line_art = $1;
}
| DynamicLineArtClass
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = GROUPITEM_DYNAMIC_LINE_ART;
    $$->a.dynamic_line_art = $1;
}
| RectangleClass
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = GROUPITEM_RECTANGLE;
    $$->a.rectangle = $1;
}
| HotspotClass
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = GROUPITEM_HOTSPOT;
    $$->a.hotspot = $1;
}
| SwitchButtonClass
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = GROUPITEM_SWITCH_BUTTON;
    $$->a.switch_button = $1;
}
| PushButtonClass
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = GROUPITEM_PUSH_BUTTON;
    $$->a.push_button = $1;
}
| TextClass
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = GROUPITEM_TEXT;
    $$->a.text = $1;
}
| EntryFieldClass
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = GROUPITEM_ENTRY_FIELD;
    $$->a.entry_field = $1;
}
| HyperTextClass
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = GROUPITEM_HYPER_TEXT;
    $$->a.hyper_text = $1;
}
| SliderClass
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = GROUPITEM_SLIDER;
    $$->a.slider = $1;
}
| TokenGroupClass
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = GROUPITEM_TOKEN_GROUP;
    $$->a.token_group = $1;
}
| ListGroupClass
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = GROUPITEM_LIST_GROUP;
    $$->a.list_group = $1;
}
| eTOK_LBRACE error eTOK_RBRACE
{
    diag_msg(eSEVERITY_INFO,"Discarding item from group", @3.first_line);
    $$ = 0;
}
;


/* Ingredient. */
InitiallyActive: eTOK_TAG_InitiallyActive eTOK_BOOLEAN
{
    $$=$2;
    if( $2 == AsnTrue )
        warn_default("InitiallyActive", @2.first_line);
}
| /* empty. */ {$$=AsnTrue;}
;
ContentHook: eTOK_TAG_ContentHook Integer
{
    $$=snaccAlloc(sizeof(*$$));
    *$$ = $2;
}
| /* empty. */ {$$=0;}
;
OriginalContent : eTOK_TAG_OriginalContent eTOK_STRING
{
    $$=snaccAlloc(sizeof(*$$));
    $$->choiceId = CONTENTBODY_INCLUDED_CONTENT;
    $$->a.included_content = snaccAlloc(sizeof(AsnOcts));
    *($$->a.included_content) = decode_string($2);
}
| eTOK_TAG_OriginalContent eTOK_TAG_ContentRef eTOK_LPAREN
                                                   eTOK_STRING
                                                   ContentSize
                                                   ContentCachePriority
                                               eTOK_RPAREN
{
    $$=snaccAlloc(sizeof(*$$));
    $$->choiceId = CONTENTBODY_REFERENCED_CONTENT;
    $$->a.referenced_content = snaccAlloc(sizeof(struct ReferencedContent));
    $$->a.referenced_content->content_reference = decode_string($4);
    $$->a.referenced_content->content_size = $5;
    $$->a.referenced_content->content_cache_priority = $6;
}
|  /* empty. */ {$$=0;}
;
Shared: eTOK_TAG_Shared eTOK_BOOLEAN
{
    if( !$2 ){
        warn_default("Shared", @2.first_line);
    }
    $$ = $2;
}
|  /* empty. */ {$$=AsnFalse;}
;

ContentSize: eTOK_TAG_ContentSize Integer
{
    $$=snaccAlloc(sizeof(*$$));
    *$$ = $2;
}
|  /* empty. */ {$$=0;}
;
ContentCachePriority: eTOK_TAG_ContentCachePriority Integer
{
    if( $2 == 127 ){
        warn_default("ContentCachePriority", @2.first_line);
        $$ = 0;
    }
    else {
        $$=snaccAlloc(sizeof(*$$));
        *$$ = $2;
    }
    
}
|  /* empty. */ {$$=0;}
;

/* Link */

LinkClass: eTOK_LBRACE eTOK_TAG_Link
                       ObjectReference
                       InitiallyActive
                       ContentHook
                       OriginalContent
                       Shared
                       eTOK_TAG_EventSource ObjectReference
                       eTOK_TAG_EventType EventTypeEnum
                       EventData
                       eTOK_TAG_LinkEffect ActionClass
           eTOK_RBRACE
{
    $$ = snaccAlloc(sizeof(*$$));

    /* ObjectReference */
    $$->rootClass = $3;
//TODO: check objectref.
    
    /* InitiallyActive */
    if( $4 == AsnFalse ) {
        $$->initially_active = snaccAlloc(sizeof(AsnBool));
        *($$->initially_active) = AsnFalse;
    }
    
    /* ContentHook */
    if( $5 )
        warn_bad_field("Link", "ContentHook", @5.first_line);
    
    /* OriginalContent */
    if( $6 )
        warn_bad_field("Link", "OriginalContent", @6.first_line);
    
    /* Shared */
    if( $7 ) {
        $$->shared = snaccAlloc(sizeof(AsnBool));
        *($$->shared) = AsnTrue;
    }
    
    /* LinkCondition */
    $$->link_condition = snaccAlloc( sizeof(struct LinkCondition));
    $$->link_condition->event_source = $9;
    $$->link_condition->event_type = $11;
    $$->link_condition->event_data = $12;
        
    /* LinkEffect */
    $$->link_effect = $14;
}
;
/* Returns snacc EventType enum for event type seen.
 * Can decode or use tokens.
 */
EventTypeEnum: eTOK_ENUM_IsAvailable {$$ = IS_AVAILABLE;}
| eTOK_ENUM_ContentAvailable         {$$ = CONTENT_AVAILABLE;}
| eTOK_ENUM_IsDeleted                {$$ = IS_DELETED;}
| eTOK_ENUM_IsRunning                {$$ = IS_RUNNING;}
| eTOK_ENUM_IsStopped                {$$ = IS_STOPPED;}
| eTOK_ENUM_UserInput                {$$ = USER_INPUT;}
| eTOK_ENUM_AnchorFired              {$$ = ANCHOR_FIRED;}
| eTOK_ENUM_TimerFired               {$$ = TIMER_FIRED;}
| eTOK_ENUM_AsynchStopped            {$$ = ASYNCH_STOPPED;}
| eTOK_ENUM_InteractionCompleted     {$$ = INTERACTION_COMPLETED;}
| eTOK_ENUM_TokenMovedFrom           {$$ = TOKEN_MOVED_FROM;}
| eTOK_ENUM_TokenMovedTo             {$$ = TOKEN_MOVED_TO;}
| eTOK_ENUM_StreamEvent              {$$ = STREAM_EVENT;}
| eTOK_ENUM_StreamPlaying            {$$ = STREAM_PLAYING;}
| eTOK_ENUM_StreamStopped            {$$ = STREAM_STOPPED;}
| eTOK_ENUM_CounterTrigger           {$$ = COUNTER_TRIGGER;}
| eTOK_ENUM_HighlightOn              {$$ = HIGHLIGHT_ON;}
| eTOK_ENUM_HighlightOff             {$$ = HIGHLIGHT_OFF;}
| eTOK_ENUM_CursorEnter              {$$ = CURSOR_ENTER;}
| eTOK_ENUM_CursorLeave              {$$ = CURSOR_LEAVE;}
| eTOK_ENUM_IsSelected               {$$ = IS_SELECTED;}
| eTOK_ENUM_IsDeselected             {$$ = IS_DESELECTED;}
| eTOK_ENUM_TestEvent                {$$ = TEST_EVENT;}
| eTOK_ENUM_FirstItemPresented       {$$ = FIRST_ITEM_PRESENTED;}
| eTOK_ENUM_LastItemPresented        {$$ = LAST_ITEM_PRESENTED;}
| eTOK_ENUM_HeadItems                {$$ = HEAD_ITEMS;}
| eTOK_ENUM_TailItems                {$$ = TAIL_ITEMS;}
| eTOK_ENUM_ItemSelected             {$$ = ITEM_SELECTED;}
| eTOK_ENUM_ItemDeselected           {$$ = ITEM_DESELECTED;}
| eTOK_ENUM_EntryFieldFull           {$$ = ENTRY_FIELD_FULL;}
| eTOK_ENUM_EngineEvent              {$$ = ENGINE_EVENT;}
| eTOK_ENUM_FocusMoved               {$$ = FOCUS_MOVED;}
| eTOK_ENUM_SliderValueChanged       {$$ = SLIDER_VALUE_CHANGED;}
;
EventData: eTOK_TAG_EventData eTOK_STRING
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = EVENTDATA_OCTETSTRING;
    $$->a.octetstring = snaccAlloc(sizeof(AsnOcts));
    *($$->a.octetstring) = decode_string($2);
}
| eTOK_TAG_EventData eTOK_BOOLEAN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = EVENTDATA_BOOLEAN;
    $$->a.boolean = $2;
}
| eTOK_TAG_EventData Integer
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = EVENTDATA_INTEGER;
    $$->a.integer = $2;
    
}
| /* empty */ {$$=0;}
;


/* Program. */

InitiallyAvailable : eTOK_TAG_InitiallyAvailable eTOK_BOOLEAN
{
    $$ = $2;
    if( $2 )
        warn_default("InitiallyAvailable", @2.first_line);
}
| /* empty */
{
    $$=AsnTrue;
}
;

/* Resident Program. */
/* ResidentProgramClass::= "{:ResidentPrg" Program "}" */
/*
  ResidentProgramClass ::= ProgramClass (WITH COMPONENTS {
                                         ..., content-hook ABSENT,
                                         original-content ABSENT})
*/
ResidentProgramClass: eTOK_LBRACE eTOK_TAG_ResidentProgram
                                  ObjectReference
                                  InvInitiallyActive
                                  ContentHook
                                  OriginalContent
                                  Shared
                                  eTOK_TAG_Name eTOK_STRING
                                  InitiallyAvailable
                      eTOK_RBRACE
{
    $$=snaccAlloc( sizeof(*$$));
    $$->rootClass = $3;
    
    /* InitiallyActive */
    $$->initially_active = snaccAlloc(sizeof(AsnBool));
    *($$->initially_active) = $4;
    
    /* ContentHook */
    if( $5 )
        warn_bad_field("ResidentProgramClass", "ContentHook", @5.first_line);
    
    /* OriginalContent */
    if( $6 )
        warn_bad_field("ResidentProgramClass", "OriginalContent", @6.first_line);

    /* Shared */
    if( $7 ) {
        $$->shared = snaccAlloc(sizeof(AsnBool));
        *($$->shared) = AsnTrue;
    }

    /* name. */
    $$->name = decode_string($9);
    
    /* initially available. */
    if( !$10 ){
        $$->initially_available = snaccAlloc(sizeof(AsnBool));
        *($$->initially_available) = AsnFalse;
    }
}
;
InvInitiallyActive: eTOK_TAG_InitiallyActive eTOK_BOOLEAN
{
    $$=$2;
    if( $2 == AsnFalse )
        warn_default("InitiallyActive (resident-program)", @2.first_line);
}
| /* empty. */ {$$=AsnFalse;}
;

RemoteProgramClass: eTOK_LBRACE eTOK_TAG_RemoteProgram
                                  ObjectReference
                                  InvInitiallyActive
                                  ContentHook
                                  OriginalContent
                                  Shared
                                  eTOK_TAG_Name eTOK_STRING
                                  InitiallyAvailable
                    eTOK_RBRACE
{
    if(!testing_mode)
        warn("RemoteProgramClass: Not included in UK/FreeSat MHEG profile", @2.first_line);

    $$ = snaccAlloc(sizeof(*$$));

    $$->rootClass = $3;

    /* InitiallyActive */
    $$->initially_active = snaccAlloc(sizeof(AsnBool));
    *($$->initially_active) = $4;

    /* ContentHook */
    if( $5 )
        warn_bad_field("ResidentProgramClass", "ContentHook", @5.first_line);
    
    /* OriginalContent */
    if( $6 )
        warn_bad_field("ResidentProgramClass", "OriginalContent", @6.first_line);

    /* Shared */
    if( $7 ) {
        $$->shared = snaccAlloc(sizeof(AsnBool));
        *($$->shared) = AsnTrue;
    }
    
    /* name. */
    $$->name = decode_string($9);
    
    /* initially available. */
    if( !$10 ){
        $$->initially_available = snaccAlloc(sizeof(AsnBool));
        *($$->initially_available) = AsnFalse;
    }
}
|                   eTOK_LBRACE eTOK_TAG_RemoteProgram
                                  ObjectReference
                                  InvInitiallyActive
                                  ContentHook
                                  OriginalContent
                                  Shared
                                  eTOK_TAG_Name eTOK_STRING
                                  InitiallyAvailable
                                  eTOK_TAG_ConnectionTag Integer
                    eTOK_RBRACE
{
    warn("Not supported: RemoteProgramClass", @10.first_line);
    $$=0;
}
;

InterchangedProgramClass: eTOK_LBRACE eTOK_TAG_InterchangedProgram
                                  ObjectReference
                                  InvInitiallyActive
                                  ContentHook
                                  OriginalContent
                                  Shared
                                  eTOK_TAG_Name eTOK_STRING
                                  InitiallyAvailable
                          eTOK_RBRACE
{
    if(!testing_mode)
        warn("InterchangedProgramClass: Not included in UK/FreeSat MHEG profile", @2.first_line);
        
    $$ = snaccAlloc(sizeof(*$$));

    $$->rootClass = $3;

    /* InitiallyActive */
    $$->initially_active = snaccAlloc(sizeof(AsnBool));
    *($$->initially_active) = $4;

    /* ContentHook */
    if( !$5 )
        warn("InterchangedProgramClass: ContentHook is mandatory for this class", @5.first_line);
    
    /* OriginalContent */
    if( !$6 )
        warn("InterchangedProgramClass: OriginalContent is mandatory for this class", @6.first_line);

    /* Shared */
    if( $7 ) {
        $$->shared = snaccAlloc(sizeof(AsnBool));
        *($$->shared) = AsnTrue;
    }
    
    /* name. */
    $$->name = decode_string($9);
    
    /* initially available. */
    if( !$10 ){
        $$->initially_available = snaccAlloc(sizeof(AsnBool));
        *($$->initially_available) = AsnFalse;
    }
}
;

PaletteClass: eTOK_LBRACE eTOK_TAG_Palette
                                  ObjectReference
                                  InitiallyActive
                                  ContentHook
                                  OriginalContent
                                  Shared
               eTOK_RBRACE
{
    if(!testing_mode)
        warn("PaletteClass: Not included in UK/FreeSat MHEG profile", @2.first_line);

    $$ = snaccAlloc(sizeof(*$$));

    $$->rootClass = $3;

    /* InitiallyActive must be true (if present). */
    if( !$4 )
        warn("PaletteClass: InitiallyActive shall be TRUE is present", @4.first_line);

    /* ContentHook */
    if( !$5 )
        warn("PaletteClass: ContentHook is mandatory for this class", @5.first_line);
    
    /* OriginalContent */
    if( !$6 )
        warn("PaletteClass: OriginalContent is mandatory for this class", @6.first_line);

    /* Shared */
    if( $7 ) {
        $$->shared = snaccAlloc(sizeof(AsnBool));
        *($$->shared) = AsnTrue;
    }
}
;

FontClass: eTOK_LBRACE eTOK_TAG_Font
                                  ObjectReference
                                  InitiallyActive
                                  ContentHook
                                  OriginalContent
                                  Shared
           eTOK_RBRACE
{	
    $$ = snaccAlloc(sizeof(*$$));

    $$->rootClass = $3;

    /* InitiallyActive: If encoded, this attribute shall be set to True */
    if( !$4 )
        warn("FontClass: InitiallyActive shall be TRUE is present", @4.first_line);

    /* ContentHook */
    if( $5 ) {
        $$->content_hook = $5;      
    }
    
    /* OriginalContent */
    $$->original_content = $6;
    if( !$6 ) {
        warn("FontClass: OriginalContent is mandatory for this class", @6.first_line);
    }

    /* Shared */
    if( $7 ) {
        $$->shared = snaccAlloc(sizeof(AsnBool));
        *($$->shared) = AsnTrue;
    }
}
;

CursorShapeClass: eTOK_LBRACE eTOK_TAG_CursorShape
                                  ObjectReference
                                  InitiallyActive
                                  ContentHook
                                  OriginalContent
                                  Shared
                  eTOK_RBRACE
{
    if(!testing_mode)
        warn("CursorShapeClass: Not included in UK/MHEG profile", @2.first_line);

    $$ = snaccAlloc(sizeof(*$$));

    $$->rootClass = $3;

    /* InitiallyActive must be true (if present). */
    if( !$4 )
        warn("CursorShapeClass: InitiallyActive shall be TRUE is present", @4.first_line);

    /* ContentHook */
    if( !$5 )
        warn("CursorShapeClass: ContentHook is mandatory for this class", @5.first_line);
    
    /* OriginalContent */
    if( !$6 )
        warn("CursorShapeClass: OriginalContent is mandatory for this class", @6.first_line);

    /* Shared */
    if( $7 ) {
        $$->shared = snaccAlloc(sizeof(AsnBool));
        *($$->shared) = AsnTrue;
    }
}
;


/* Variable. */
OrigValue: eTOK_BOOLEAN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ORIGINALVALUE_BOOLEAN;
    $$->a.boolean = $1;
}
| Integer
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ORIGINALVALUE_INTEGER;
    $$->a.integer = $1;
}
| eTOK_STRING
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ORIGINALVALUE_OCTETSTRING;
    $$->a.octetstring = snaccAlloc(sizeof(AsnOcts));
    *($$->a.octetstring) = decode_string($1);
}
| eTOK_TAG_ObjectReferenceValue ObjectReference
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ORIGINALVALUE_OBJECT_REFERENCE;
    $$->a.object_reference = $2;
}
| eTOK_TAG_ContentRef eTOK_STRING
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ORIGINALVALUE_CONTENT_REFERENCE;
    $$->a.content_reference = snaccAlloc(sizeof(ContentReference));
    *($$->a.content_reference) = decode_string($2);
}
;

BooleanVariableClass: eTOK_LBRACE eTOK_TAG_BooleanVariable
                                   ObjectReference
                                   InitiallyActive
                                   ContentHook
                                   OriginalContent
                                   Shared
                                   eTOK_TAG_OriginalValue OrigValue
                      eTOK_RBRACE
{
    $$ = snaccAlloc(sizeof(*$$));

    $$->rootClass = $3;

    /* InitiallyActive must be true. */
    if( !$4 )
        warn_bad_field("Variable", "InitiallyActive=FALSE", @4.first_line);

    if( $5 )
        warn_bad_field("Variable", "ContentHook", @5.first_line);
    
    /* OriginalContent */
    if( $6 )
        warn_bad_field("Variable", "OriginalContent", @6.first_line);

    /* Shared */
    if( $7 ) {
        $$->shared = snaccAlloc(sizeof(AsnBool));
        *($$->shared) = AsnTrue;
    }

    /* Original Value. */
    $$->original_value = $9;
}
;

IntegerVariableClass: eTOK_LBRACE eTOK_TAG_IntegerVariable
                                   ObjectReference
                                   InitiallyActive
                                   ContentHook
                                   OriginalContent
                                   Shared
                                   eTOK_TAG_OriginalValue OrigValue
                      eTOK_RBRACE
{
    $$ = snaccAlloc(sizeof(*$$));

    $$->rootClass = $3;

    /* InitiallyActive must be true. */
    if( !$4 )
        warn_bad_field("Variable", "InitiallyActive=FALSE", @4.first_line);

    if( $5 )
        warn_bad_field("Variable", "ContentHook", @5.first_line);
    
    /* OriginalContent */
    if( $6 )
        warn_bad_field("Variable", "OriginalContent", @6.first_line);

    /* Shared */
    if( $7 ) {
        $$->shared = snaccAlloc(sizeof(AsnBool));
        *($$->shared) = AsnTrue;
    }

    /* Original Value. */
    $$->original_value = $9;
}
;

OctetStringVariableClass: eTOK_LBRACE eTOK_TAG_OctetStringVariable
                                   ObjectReference
                                   InitiallyActive
                                   ContentHook
                                   OriginalContent
                                   Shared
                                   eTOK_TAG_OriginalValue OrigValue
                      eTOK_RBRACE
{
    $$ = snaccAlloc(sizeof(*$$));

    $$->rootClass = $3;

    /* InitiallyActive must be true. */
    if( !$4 )
        warn_bad_field("Variable", "InitiallyActive=FALSE", @4.first_line);

    if( $5 )
        warn_bad_field("Variable", "ContentHook", @5.first_line);
    
    /* OriginalContent */
    if( $6 )
        warn_bad_field("Variable", "OriginalContent", @6.first_line);

    /* Shared */
    if( $7 ) {
        $$->shared = snaccAlloc(sizeof(AsnBool));
        *($$->shared) = AsnTrue;
    }

    /* Original Value. */
    $$->original_value = $9;
}
;

ObjectRefVariableClass: eTOK_LBRACE eTOK_TAG_ObjectRefVariable
                                   ObjectReference
                                   InitiallyActive
                                   ContentHook
                                   OriginalContent
                                   Shared
                                   eTOK_TAG_OriginalValue OrigValue
                      eTOK_RBRACE
{
    $$ = snaccAlloc(sizeof(*$$));

    $$->rootClass = $3;

    /* InitiallyActive must be true. */
    if( !$4 )
        warn_bad_field("Variable", "InitiallyActive=FALSE", @4.first_line);

    if( $5 )
        warn_bad_field("Variable", "ContentHook", @5.first_line);
    
    /* OriginalContent */
    if( $6 )
        warn_bad_field("Variable", "OriginalContent", @6.first_line);

    /* Shared */
    if( $7 ) {
        $$->shared = snaccAlloc(sizeof(AsnBool));
        *($$->shared) = AsnTrue;
    }

    /* Original Value. */
    $$->original_value = $9;
}
;

ContentRefVariableClass: eTOK_LBRACE eTOK_TAG_ContentRefVariable
                                   ObjectReference
                                   InitiallyActive
                                   ContentHook
                                   OriginalContent
                                   Shared
                                   eTOK_TAG_OriginalValue OrigValue
                      eTOK_RBRACE
{
    $$ = snaccAlloc(sizeof(*$$));

    $$->rootClass = $3;

    /* InitiallyActive must be true. */
    if( !$4 )
        warn_bad_field("Variable", "InitiallyActive=FALSE", @4.first_line);

    if( $5 )
        warn_bad_field("Variable", "ContentHook", @5.first_line);
    
    /* OriginalContent */
    if( $6 )
        warn_bad_field("Variable", "OriginalContent", @6.first_line);

    /* Shared */
    if( $7 ) {
        $$->shared = snaccAlloc(sizeof(AsnBool));
        *($$->shared) = AsnTrue;
    }

    /* Original Value. */
    $$->original_value = $9;
}
;

/* Token Manager. */

TokenManager: eTOK_TAG_MovementTable eTOK_LPAREN MovementList eTOK_RPAREN
{$$=$3;}
| /* empty */ {$$=0;}
;
/* Be careful, is a list of list pointers. */
MovementList: Movement MovementList
{
    /* The prepend will create a node and return pointer to the data.
     */
    *((AsnList **)AsnListPrepend($2)) = $1;
    $$ = $2;
}
| Movement
{
    /* This should be the last in the list (first reduce) so create the AsnList
     * and the other alternate can prepend to it.
     */
    $$ = AsnListNew(sizeof($1));
    /* The prepend will create a node and return pointer to the data.
     */
    *((AsnList **)AsnListPrepend($$)) = $1;
}
;
Movement: eTOK_LPAREN IntegerList eTOK_RPAREN {$$=$2;}
;
IntegerList: Integer IntegerList
{
    AsnInt *p;
    /* The prepend will create a node and return pointer to the data.
     */
    p = snaccAlloc(sizeof(*p));
    *p = $1;
    *((int **)AsnListPrepend($2)) = p;
    $$ = $2;
}
| Integer
{
    AsnInt *p;
    /* This should be the last in the list (first reduce) so create the AsnList
     * and the other alternate can prepend to it.
     */
    p = snaccAlloc(sizeof(*p));
    *p = $1;
    $$ = AsnListNew(sizeof($1));
    /* The prepend will create a node and return pointer to the data.
     */
    *((int **)AsnListPrepend($$)) = p;
}
;

/* TokenGroup */
TokenGroupClass : eTOK_LBRACE eTOK_TAG_TokenGroup
                              ObjectReference
                              InitiallyActive
                              ContentHook
                              OriginalContent
                              Shared
                              TokenManager
                              TokenGroupItems
                              NoTokenActionSlots
                  eTOK_RBRACE
{
    $$ = snaccAlloc(sizeof(*$$));

    $$->rootClass = $3;
    
    /* InitiallyActive */
    if( $4 == AsnFalse ) {
        $$->initially_active = snaccAlloc(sizeof(AsnBool));
        *($$->initially_active) = AsnFalse;
    }
    
    /* ContentHook */
    if( $5 )
        warn_bad_field("TokenGroup", "ContentHook", @5.first_line);
    
    /* OriginalContent */
    if( $6 )
        warn_bad_field("TokenGroup", "OriginalContent", @6.first_line);
    
    /* Shared */
    if( $7 ) {
        $$->shared = snaccAlloc(sizeof(AsnBool));
        *($$->shared) = AsnTrue;
    }

    $$->movement_table = $8;
    $$->token_group_items = $9;
    $$->no_token_action_slots = $10;
}
;
TokenGroupItems: eTOK_TAG_TokenGroupItems eTOK_LPAREN
                                            TokenGroupItemList
                                          eTOK_RPAREN
{$$ = $3;}
| /* empty */ {$$=0;}
;


NoTokenActionSlots : eTOK_TAG_NoTokenActionSlots eTOK_LPAREN
                                                 ActionSlotList
                                                 eTOK_RPAREN
{
    $$ = $3;
}
| /* empty */ {$$ = 0;}
;
ActionSlotList: ActionSlot ActionSlotList
{
    /* The prepend will create a node and return pointer to the data.
     */
    *((ActionSlot **)AsnListPrepend($2)) = $1;
    $$ = $2;
}
| ActionSlot
{
    /* This should be the last in the list (first reduce) so create the AsnList
     * and the other alternate can prepend to it.
     */
    $$ = AsnListNew(sizeof($1));
    /* The prepend will create a node and return pointer to the data.
     */
    *((ActionSlot **)AsnListPrepend($$)) = $1;
}
;
ActionSlot: ActionClass
{
    $$=snaccAlloc(sizeof(*$$));
    $$->choiceId = ACTIONSLOT_ACTION_CLASS;
    $$->a.action_class = $1;
}
| eTOK_NULL
{
    $$=snaccAlloc(sizeof(*$$));
    $$->choiceId = ACTIONSLOT_NULL;
}
;
TokenGroupItemList: TokenGroupItem TokenGroupItemList
{
    /* The prepend will create a node and return pointer to the data.
     */
    *((TokenGroupItem **)AsnListPrepend($2)) = $1;
    $$ = $2;
}
| TokenGroupItem
{
    /* This should be the last in the list (first reduce) so create the AsnList
     * and the other alternate can prepend to it.
     */
    $$ = AsnListNew(sizeof($1));
    /* The prepend will create a node and return pointer to the data.
     */
    *((TokenGroupItem **)AsnListPrepend($$)) = $1;
}
;
TokenGroupItem: eTOK_LPAREN ObjectReference ActionSlots eTOK_RPAREN
{
    $$=snaccAlloc(sizeof(*$$));
    $$->a_visible = $2;
    $$->action_slots = $3;
}
;
ActionSlots: eTOK_TAG_ActionSlots eTOK_LPAREN ActionSlotList eTOK_RPAREN
{
    $$ = $3;
}
| /* empty. */ {$$=0;}
;
            
/* ListGroup */
ListGroupClass:   eTOK_LBRACE eTOK_TAG_ListGroup
                              ObjectReference
                              InitiallyActive
                              ContentHook
                              OriginalContent
                              Shared
                              TokenManager
                              TokenGroupItems
                              NoTokenActionSlots
                              eTOK_TAG_Positions
                              eTOK_LPAREN
                              PositionList
                              eTOK_RPAREN
                              WrapAround
                              MultipleSelection
                  eTOK_RBRACE
{
    $$ = snaccAlloc(sizeof(*$$));

    $$->rootClass = $3;
    
    /* InitiallyActive */
    if( $4 == AsnFalse ) {
        $$->initially_active = snaccAlloc(sizeof(AsnBool));
        *($$->initially_active) = AsnFalse;
    }
    
    /* ContentHook */
    if( $5 )
        warn_bad_field("ListGroup", "ContentHook", @5.first_line);
    
    /* OriginalContent */
    if( $6 )
        warn_bad_field("ListGroup", "OriginalContent", @6.first_line);
    
    /* Shared */
    if( $7 ) {
        $$->shared = snaccAlloc(sizeof(AsnBool));
        *($$->shared) = AsnTrue;
    }

    $$->movement_table = $8;
    $$->token_group_items = $9;
    $$->no_token_action_slots = $10;

    $$->positions = $13; 
    if( $15 ) {
        $$->wrap_around = snaccAlloc(sizeof(AsnBool));
        *($$->wrap_around) = AsnTrue;
    }
    if( $16 ) {
        $$->multiple_selection = snaccAlloc(sizeof(AsnBool));
        *($$->multiple_selection) = AsnTrue;
    }
}
;

PositionList: Position PositionList
{
    /* The prepend will create a node and return pointer to the data.
     */
    *((XYPosition **)AsnListPrepend($2)) = $1;
    $$ = $2;
}
| Position 
{
    /* This should be the last in the list (first reduce) so create the AsnList
     * and the other alternate can prepend to it.
     */
    $$ = AsnListNew(sizeof($1));
    /* The prepend will create a node and return pointer to the data.
     */
    *((XYPosition **)AsnListPrepend($$)) = $1;
}
;
Position: eTOK_LPAREN Integer Integer eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));    
    $$->x_position = $2;
    $$->y_position = $3;
}
;
WrapAround: eTOK_TAG_WrapAround eTOK_BOOLEAN
{
    if( !$2 ){
        warn_default("WrapAround", @2.first_line);
    }
    $$ = $2;
}
|  /* empty. */ {$$=AsnFalse;}
;
MultipleSelection: eTOK_TAG_MultipleSelection eTOK_BOOLEAN
{
    if( !$2 ){
        warn_default("MultipleSelection", @2.first_line);
    }
    $$ = $2;
}
|  /* empty. */ {$$=AsnFalse;}
;

/* Visible */
OriginalPosition: eTOK_TAG_OriginalPosition Integer Integer
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->x_position = $2;
    $$->y_position = $3;
    if( $2 == 0 && $3 == 0 )
        warn_default("OriginalPosition", @2.first_line);
}
|
{
    $$=0;
}
;
OriginalPaletteRef: eTOK_TAG_OriginalPaletteRef ObjectReference {$$=$2;}
| {$$=0;}
;


/* Bitmap */
BitmapClass: eTOK_LBRACE eTOK_TAG_Bitmap
                          ObjectReference
                          InitiallyActive
                          ContentHook
                          OriginalContent
                          Shared
                          eTOK_TAG_OriginalBoxSize
                          Integer
                          Integer
                          OriginalPosition
                          OriginalPaletteRef
                          Tiling
                          OriginalTransparency
             eTOK_RBRACE
{
    $$ = snaccAlloc(sizeof(*$$));

    $$->rootClass = $3;
    
    /* InitiallyActive */
    if( $4 == AsnFalse ) {
        $$->initially_active = snaccAlloc(sizeof(AsnBool));
        *($$->initially_active) = AsnFalse;
    }
    
    /* ContentHook */
    if( $5 ) {
        $$->content_hook = $5;
    }
       
    /* OriginalContent */
    $$->original_content = $6;
    if( !$6 ) {
        warn("need OriginalContent for bitmap.", @2.first_line);
    }
    
    /* Shared */
    if( $7 ) {
        $$->shared = snaccAlloc(sizeof(AsnBool));
        *($$->shared) = AsnTrue;
    }

    /* OriginalBoxSize */
    $$->original_box_size = snaccAlloc(sizeof(OriginalBoxSize));
    $$->original_box_size->x_length = $9;
    $$->original_box_size->y_length = $10;

    /* OriginalPosition */
    $$->original_position = $11;

    /* OriginalPaletteRef */
    $$->original_palette_ref = $12;
    
    /* Tiling */
    if( $13 )
    {
        $$->tiling = snaccAlloc(sizeof(AsnBool));
        *($$->tiling) = AsnTrue;
    }
    
    /* OriginalTransparency */
    if( $14 != 0 )
    {
        $$->original_transparency = snaccAlloc(sizeof(AsnInt));
        *($$->original_transparency) = $14;
    }
}
;
Tiling: eTOK_TAG_Tiling eTOK_BOOLEAN
{
    $$=$2;
    if( !$2 )
        warn_default("Tiling", @2.first_line);
}
| /* empty. */ {$$=AsnFalse;}
;
OriginalTransparency: eTOK_TAG_OriginalTransparency Integer
{
    $$=$2;
    if( $2 == 0 )
        warn_default("OriginalTransparency", @2.first_line);
}
| /* empty. */ {$$=0;}
;

/* LineArt */
LineArtClass: eTOK_LBRACE eTOK_TAG_LineArt LineArtBody eTOK_RBRACE
{
    $$=$3;
    if( !$$->original_content )
        warn("need OriginalContent for LineArt.", @2.first_line);
}
;

/* returns a LineArtClass structure that can be cast and checked for overrides.
 */
LineArtBody:           ObjectReference
                       InitiallyActive
                       ContentHook
                       OriginalContent
                       Shared
                       eTOK_TAG_OriginalBoxSize
                       Integer
                       Integer
                       OriginalPosition
                       OriginalPaletteRef
                       BorderedBoundingBox
                       OriginalLineWidth
                       OriginalLineStyle
                       OriginalRefLineColour
                       OriginalRefFillColour
{
    $$ = snaccAlloc(sizeof(*$$));

    $$->rootClass = $1;
    
    /* InitiallyActive */
    if( $2 == AsnFalse ) {
        $$->initially_active = snaccAlloc(sizeof(AsnBool));
        *($$->initially_active) = AsnFalse;
    }
    
    /* ContentHook */
    if( $3 ) {
        $$->content_hook = $3;
    }
       
    /* OriginalContent */
    $$->original_content = $4;
    
    /* Shared */
    if( $5 ) {
        $$->shared = snaccAlloc(sizeof(AsnBool));
        *($$->shared) = AsnTrue;
    }

    /* OriginalBoxSize */
    $$->original_box_size = snaccAlloc(sizeof(OriginalBoxSize));
    $$->original_box_size->x_length = $7;
    $$->original_box_size->y_length = $8;

    /* OriginalPosition */
    $$->original_position = $9;

    /* OriginalPaletteRef */
    $$->original_palette_ref = $10;

    /* BorderedBoundingBox */
    if( !$11 ){
        $$->bordered_bounding_box = snaccAlloc(sizeof(AsnBool));
        *($$->bordered_bounding_box) = AsnFalse;
    }
    /* OriginalLineWidth */
    if( $12 != 1 ){
        $$->original_line_width = snaccAlloc(sizeof(AsnInt));
        *($$->original_line_width) = $12;
    }
    /* OriginalLineStyle */
    if( $13 != 1 ){
        $$->original_line_style = snaccAlloc(sizeof(LineArtClassInt));
        *($$->original_line_style) = $13;
    }
    /* OriginalRefLineColour */
    $$->original_ref_line_colour = $14;
    /* OriginalRefFillColour */
    $$->original_ref_fill_colour = $15;
}
;
BorderedBoundingBox: eTOK_TAG_BorderedBoundingBox eTOK_BOOLEAN
{
    if( $2 ){
        warn_default("BorderedBoundingBox", @2.first_line);
        $$ = AsnTrue;
    }
    else
        $$ = AsnFalse;
}
| /* empty */ {$$=AsnTrue;}
;
OriginalLineWidth: eTOK_TAG_OriginalLineWidth Integer
{
    $$ = $2;
    if( $$ == 1 )
        warn_default("OriginalLineWidth", @2.first_line);
}
| /* empty */ {$$=1;}
;
OriginalLineStyle: eTOK_TAG_OriginalLineStyle Integer
{
    $$ = $2;
    if( $$ == SOLID )               /* DEFAULT solid (1) */
        warn_default("OriginalLineStyle", @2.first_line);
}
| /* empty */ {$$=SOLID;}
;
OriginalRefLineColour: eTOK_TAG_OriginalRefLineColour Colour {$$=$2;}
| /* empty */ {$$=0;}
;
OriginalRefFillColour: eTOK_TAG_OriginalRefFillColour Colour {$$=$2;}
| /* empty */ {$$=0;}
;

/* Rectangle */
RectangleClass: eTOK_LBRACE eTOK_TAG_Rectangle LineArtBody eTOK_RBRACE
{
    $$=(RectangleClass *)$3;
    if( $$->content_hook ){
        warn_bad_field("Rectangle", "content_hook", @2.first_line);
    }
    if( $$->original_content ){
        warn_bad_field("Rectangle", "original_content", @2.first_line);
    }
    if( $$->bordered_bounding_box ){
        warn_bad_field("Rectangle", "bordered_bounding_box", @2.first_line);
    }
    
}
;
/* DynamicLineArt */
DynamicLineArtClass: eTOK_LBRACE eTOK_TAG_DynamicLineArt
                                  LineArtBody
                     eTOK_RBRACE
{
    $$=(DynamicLineArtClass *)$3;
    if( $$->content_hook ){
        warn_bad_field("DynamicLineArt", "content_hook", @2.first_line);
    }
    if( $$->original_content ){
        warn_bad_field("DynamicLineArt", "original_content", @2.first_line);
    }
}
;

/* Text */
TextClass: eTOK_LBRACE eTOK_TAG_Text
                       ObjectReference
                       InitiallyActive
                       ContentHook
                       OriginalContent
                       Shared
                       eTOK_TAG_OriginalBoxSize
                       Integer
                       Integer
                       OriginalPosition
                       OriginalPaletteRef
                       OriginalFont
                       OptionalFontAttributes
                       OptionalTextColour
                       OptionalBackgroundColour
                       OptionalCharacterSet
                       HorizontalJustification
                       VerticalJustification
                       LineOrientation
                       StartCorner
                       TextWrapping
           eTOK_RBRACE
{
    $$ = snaccAlloc(sizeof(*$$));

    $$->rootClass = $3;
    
    /* InitiallyActive */
    if( $4 == AsnFalse ) {
        $$->initially_active = snaccAlloc(sizeof(AsnBool));
        *($$->initially_active) = AsnFalse;
    }
    
    /* ContentHook */
    if( $5 ) {
        $$->content_hook = $5;
    }
       
    /* OriginalContent */
    $$->original_content = $6;
    if( !$6 ) {
        warn("need OriginalContent for text.", @2.first_line);
    }
    
    /* Shared */
    if( $7 ) {
        $$->shared = snaccAlloc(sizeof(AsnBool));
        *($$->shared) = AsnTrue;
    }

    /* OriginalBoxSize */
    $$->original_box_size = snaccAlloc(sizeof(OriginalBoxSize));
    $$->original_box_size->x_length = $9;
    $$->original_box_size->y_length = $10;

    /* OriginalPosition */
    $$->original_position = $11;

    /* OriginalPaletteRef */
    $$->original_palette_ref = $12;
    /* OriginalFont */
    $$->original_font = $13;
    /* OptionalFontAttributes */
    $$->font_attributes = $14;
    /* OptionalTextColour */
    $$->text_colour = $15;
    /* OptionalBackgroundColour */
    $$->background_colour = $16;
    /* OptionalCharacterSet */
    $$->character_set = $17;
    /* HorizontalJustification */
    if( $18 != START ){
        $$->horizontal_justification = snaccAlloc(sizeof(Justification));
        *($$->horizontal_justification) = $18;
    }
    /* VerticalJustification */
    if( $19 != START ){
        $$->vertical_justification = snaccAlloc(sizeof(Justification));
        *($$->vertical_justification) = $19;
    }
    /* LineOrientation */
    if( $20 != HORIZONTAL ){
        $$->line_orientation = snaccAlloc(sizeof(LineOrientation));
        *($$->line_orientation) = $20;
    }
    /* StartCorner */
    if( $21 != UPPER_LEFT ){
        $$->start_corner = snaccAlloc(sizeof(StartCorner));
        *($$->start_corner) = $21;
    }
    /* TextWrapping */
    if( $22 ){
        $$->text_wrapping = snaccAlloc(sizeof(AsnBool));
        *($$->text_wrapping) = AsnTrue;
    }
}
;
OriginalFont: eTOK_TAG_OriginalFont FontBody {$$=$2;}
| /* empty */ {$$=0;}
;
OptionalFontAttributes: eTOK_TAG_FontAttributes eTOK_STRING
{$$=decode_string($2);}
| /* empty */ {$$.octetLen = 0; $$.octs = 0;}
;
OptionalTextColour: eTOK_TAG_TextColour Colour {$$=$2;}
| /* empty */ {$$=0;}
;
OptionalBackgroundColour: eTOK_TAG_BackgroundColour Colour {$$=$2;}
| /* empty */ {$$=0;}
;
OptionalCharacterSet: eTOK_TAG_CharacterSet Integer
{
    $$=snaccAlloc(sizeof(*$$));
    *$$ = $2;
}
| /* empty */ {$$=0;}
;
HorizontalJustification: eTOK_TAG_HorizontalJustification JustificationEnum
{
    $$=$2;
    if( $$ == START )
        warn_default("HorizontalJustification", @2.first_line);
}
| /* empty */ {$$=START;}
;
VerticalJustification: eTOK_TAG_VerticalJustification JustificationEnum
{
    $$=$2;
    if( $$ == START )
        warn_default("VerticalJustification", @2.first_line);
}
| /* empty */ {$$=START;}
;
LineOrientation: eTOK_TAG_LineOrientation LineOrientationEnum
{
    $$=$2;
    if( $$ == HORIZONTAL )
        warn_default("LineOrientation", @2.first_line);
}
| /* empty */ {$$=HORIZONTAL;}
;
StartCorner: eTOK_TAG_StartCorner StartCornerEnum
{
    $$=$2;
    if( $$ == UPPER_LEFT )
        warn_default("StartCorner", @2.first_line);
}
| /* empty */ {$$=UPPER_LEFT;}
;
TextWrapping: eTOK_TAG_TextWrapping eTOK_BOOLEAN
{
    $$=$2;
    if( !$$ )
        warn_default("TextWrapping", @2.first_line);
}
| /* empty */ {$$=AsnFalse;}
;
JustificationEnum: eTOK_ENUM_start {$$ = START;}
| eTOK_ENUM_end                    {$$ = END;}
| eTOK_ENUM_centre                 {$$ = CENTRE;}
| eTOK_ENUM_justified              {$$ = JUSTIFIED;}
;
LineOrientationEnum: eTOK_ENUM_vertical {$$ = VERTICAL;}
| eTOK_ENUM_horizontal              {$$ = HORIZONTAL;}
;
StartCornerEnum: eTOK_ENUM_upper_left {$$ = UPPER_LEFT;}
| eTOK_ENUM_upper_right               {$$ = UPPER_RIGHT;}
| eTOK_ENUM_lower_left                {$$ = LOWER_LEFT;}
| eTOK_ENUM_lower_right               {$$ = LOWER_RIGHT;}
;

/* Stream */
StreamClass: eTOK_LBRACE eTOK_TAG_Stream
                       ObjectReference
                       InitiallyActive
                       ContentHook
                       OriginalContent
                       Shared
                       Multiplex
                       Storage
                       Looping
             eTOK_RBRACE
{
    $$ = snaccAlloc(sizeof(*$$));

    $$->rootClass = $3;
    
    /* InitiallyActive */
    if( $4 == AsnFalse ) {
        $$->initially_active = snaccAlloc(sizeof(AsnBool));
        *($$->initially_active) = AsnFalse;
    }
    
    /* ContentHook */
    if( $5 ) {
        $$->content_hook = $5;
    }
       
    /* OriginalContent */
    $$->original_content = $6;
    if( !$6 ) {
        warn("need OriginalContent for stream.", @2.first_line);
    }
    
    /* Shared */
    if( $7 ) {
        $$->shared = snaccAlloc(sizeof(AsnBool));
        *($$->shared) = AsnTrue;
    }

    /* StreamComponentList */
    $$->multiplex = $8;
    /* Storage */
    if( $9 != STREAM){
        $$->storage = snaccAlloc(sizeof(Storage));
        *($$->storage) = $9;
    }
    /* Looping */
    if( $10 != 1 ){
        $$->looping = snaccAlloc(sizeof(StreamClassInt));
        *($$->looping) = $10;
    }    
}
;

Multiplex:  eTOK_TAG_Multiplex eTOK_LPAREN StreamComponentList eTOK_RPAREN
{
    $$ = $3;
}
| /* empty */
{
    $$ = 0;
}
;
StreamComponentList: StreamComponent StreamComponentList
{
    /* The prepend will create a node and return pointer to the data.
     */
    *((StreamComponent **)AsnListPrepend($2)) = $1;
    $$ = $2;
}
|  StreamComponent
{
    /* This should be the last in the list (first reduce) so create the AsnList
     * and the other alternate can prepend to it.
     */
    $$ = AsnListNew(sizeof($1));
    /* The prepend will create a node and return pointer to the data.
     */
    *((StreamComponent **)AsnListPrepend($$)) = $1;
}
;
StreamComponent : AudioClass
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = STREAMCOMPONENT_AUDIO;
    $$->a.audio = $1;
}
| VideoClass
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = STREAMCOMPONENT_VIDEO;
    $$->a.video = $1;
}
| RTGraphicsClass
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = STREAMCOMPONENT_RTGRAPHICS;
    $$->a.rtgraphics = $1;
}
;
Storage :eTOK_TAG_Storage StorageEnum
{
    $$=$2;
    if( $2 == STREAM )
        warn_default("Storage", @2.first_line);
}
| /* empty. */ {$$=STREAM;}
;
StorageEnum: eTOK_ENUM_memory   {$$=MEMORY}
| eTOK_ENUM_stream              {$$=STREAM}
;
Looping: eTOK_TAG_Looping Integer
{
    $$=$2;
    if( $$ == 1 )
        warn_default("Looping", @2.first_line);
}
| {$$=1;}
;

/* Audio */
AudioClass: eTOK_LBRACE eTOK_TAG_Audio
                       ObjectReference
                       InitiallyActive
                       ContentHook
                       OriginalContent
                       Shared
                       eTOK_TAG_ComponentTag Integer
                       OriginalVolume
            eTOK_RBRACE
{
    $$ = snaccAlloc(sizeof(*$$));

    $$->rootClass = $3;
    
    /* InitiallyActive */
    if( $4 == AsnFalse ) {
        $$->initially_active = snaccAlloc(sizeof(AsnBool));
        *($$->initially_active) = AsnFalse;
    }
    
    /* ContentHook */
    if( $5 ) 
        warn_bad_field("Audio", "ContentHook", @5.first_line);
       
    /* OriginalContent */
    $$->original_content = $6;
    if( $6 )
        warn_bad_field("Audio", "OriginalContent", @6.first_line);
    
    /* Shared */
    if( $7 )
        warn_bad_field("Audio", "Shared", @7.first_line);

    /* ComponentTag */
    $$->component_tag = $9;
        
    /* OriginalVolume */
    if( $10 != 0 ){
        $$->original_volume = snaccAlloc(sizeof(AsnInt));
        *($$->original_volume) = $10;
    }
}
;
OriginalVolume: eTOK_TAG_OriginalVolume Integer
{
    $$=$2;
    if( $$ == 0 )
        warn_default("OriginalVolume", @2.first_line);
}
| /* empty */ {$$=0;}
;

/* Video */
VideoClass: eTOK_LBRACE eTOK_TAG_Video
                       ObjectReference
                       InitiallyActive
                       ContentHook
                       OriginalContent
                       Shared
                       eTOK_TAG_OriginalBoxSize
                       Integer
                       Integer
                       OriginalPosition
                       OriginalPaletteRef
                       eTOK_TAG_ComponentTag Integer
                       Termination
           eTOK_RBRACE
{
    $$ = snaccAlloc(sizeof(*$$));

    $$->rootClass = $3;
    
    /* InitiallyActive */
    if( $4 == AsnFalse ) {
        $$->initially_active = snaccAlloc(sizeof(AsnBool));
        *($$->initially_active) = AsnFalse;
    }
    
    /* ContentHook */
    if( $5 ) 
        warn_bad_field("Video", "ContentHook", @5.first_line);
       
    /* OriginalContent */
    $$->original_content = $6;
    if( $6 )
        warn_bad_field("Video", "OriginalContent", @6.first_line);
    
    /* Shared */
    if( $7 )
        warn_bad_field("Video", "Shared", @7.first_line);
       
    /* OriginalBoxSize */
    $$->original_box_size = snaccAlloc(sizeof(OriginalBoxSize));
    $$->original_box_size->x_length = $9;
    $$->original_box_size->y_length = $10;

    /* OriginalPosition */
    $$->original_position = $11;

    /* OriginalPaletteRef */
    $$->original_palette_ref = $12;
    if( $12 )
        warn_bad_field("Video", "original_palette_ref", @12.first_line);

    /* ComponentTag */
    $$->component_tag = $14;

    /* Termination. */
    if( $15 != DISAPPEAR ){
        $$->termination = snaccAlloc(sizeof(Termination));
        *($$->termination) = $15;
    }
    
}
;
Termination: eTOK_TAG_Termination TerminationEnum
{
    $$ = $2;
    if( $$ == DISAPPEAR )
        warn_default("Termination", @2.first_line);
}
| {$$=DISAPPEAR;}
;
TerminationEnum: eTOK_ENUM_freeze {$$ = FREEZE;}
| eTOK_ENUM_disappear               {$$ = DISAPPEAR;}
;
/* RTGraphics */
RTGraphicsClass: eTOK_LBRACE eTOK_TAG_RTGraphics
                       ObjectReference
                       InitiallyActive
                       ContentHook
                       OriginalContent
                       Shared
                       eTOK_TAG_OriginalBoxSize
                       Integer
                       Integer
                       OriginalPosition
                       OriginalPaletteRef
                       eTOK_TAG_ComponentTag Integer
                       Termination
           eTOK_RBRACE
{
    $$ = snaccAlloc(sizeof(*$$));

    $$->rootClass = $3;
    
    /* InitiallyActive */
    if( $4 == AsnFalse ) {
        $$->initially_active = snaccAlloc(sizeof(AsnBool));
        *($$->initially_active) = AsnFalse;
    }
    
    /* ContentHook */
    if( $5 ) 
        warn_bad_field("RTGraphics", "ContentHook", @5.first_line);
       
    /* OriginalContent */
    $$->original_content = $6;
    if( $6 )
        warn_bad_field("RTGraphics", "OriginalContent", @6.first_line);
    
    /* Shared */
    if( $7 )
        warn_bad_field("RTGraphics", "Shared", @7.first_line);
       
    /* OriginalBoxSize */
    $$->original_box_size = snaccAlloc(sizeof(OriginalBoxSize));
    $$->original_box_size->x_length = $9;
    $$->original_box_size->y_length = $10;

    /* OriginalPosition */
    $$->original_position = $11;

    /* OriginalPaletteRef */
    $$->original_palette_ref = $12;

    /* ComponentTag */
    $$->component_tag = $14;

    /* Termination. */
    if( $15 != DISAPPEAR ){
        $$->termination = snaccAlloc(sizeof(Termination));
        *($$->termination) = $15;
    }    
}
;

/* Interactible */
EngineResp: eTOK_TAG_EngineResp eTOK_BOOLEAN
{
    $$=$2;
    if( $$ )
        warn_default("EngineResp", @2.first_line);
}
| /* empty. */ {$$=AsnTrue;}
;
HighlightRefColour :   eTOK_TAG_HighlightRefColour Colour {$$ = $2;}
| /* empty. */ {$$=0;}
;


/* Slider Class */
SliderClass: eTOK_LBRACE eTOK_TAG_Slider
                       ObjectReference
                       InitiallyActive
                       ContentHook
                       OriginalContent
                       Shared
                       eTOK_TAG_OriginalBoxSize
                       Integer
                       Integer
                       OriginalPosition
                       OriginalPaletteRef
                       EngineResp
                       HighlightRefColour
                       eTOK_TAG_Orientation OrientationEnum
                       eTOK_TAG_MaxValue Integer
                       MinValue
                       InitialValue
                       InitialPortion
                       StepSize
                       SliderStyle
                       SliderRefColour
             eTOK_RBRACE
{
        $$ = snaccAlloc(sizeof(*$$));

    $$->rootClass = $3;
    
    /* InitiallyActive */
    if( $4 == AsnFalse ) {
        $$->initially_active = snaccAlloc(sizeof(AsnBool));
        *($$->initially_active) = AsnFalse;
    }
    
    /* ContentHook */
    if( $5 ) 
        warn_bad_field("Slider", "ContentHook", @5.first_line);
       
    /* OriginalContent */
    $$->original_content = $6;
    if( $6 )
        warn_bad_field("Slider", "OriginalContent", @6.first_line);
    
    /* Shared */
    if( $7 ) {
        $$->shared = snaccAlloc(sizeof(AsnBool));
        *($$->shared) = AsnTrue;
    }
       
    /* OriginalBoxSize */
    $$->original_box_size = snaccAlloc(sizeof(OriginalBoxSize));
    $$->original_box_size->x_length = $9;
    $$->original_box_size->y_length = $10;

    /* OriginalPosition */
    $$->original_position = $11;

    /* OriginalPaletteRef */
    $$->original_palette_ref = $12;

    /* EngineResp */
    if( !$13 ){
        $$->engine_resp = snaccAlloc(sizeof(AsnBool));
        *($$->engine_resp) = $13;
    }
    /* HighlightRefColour */
    $$->highlight_ref_colour = $14;
    /* Orientation */
    $$->orientation = $16;
    /* MaxValue */
    $$->max_value = $18;
    /* MinValue */
    if( $19 != 1 ){
        $$->min_value = snaccAlloc(sizeof(AsnInt));
        *($$->min_value) = $19;
    }
    /* InitialValue */
    $$->initial_value = $20;
    /* InitialPortion */
    $$->initial_portion = $21;
    /* StepSize */
    if( $22 != 1 ){
        $$->step_size = snaccAlloc(sizeof(AsnInt));
        *($$->step_size) = $22;
    }
    /* SliderStyle */
    if( $23 != NORMAL ){
        $$->slider_style = snaccAlloc(sizeof(SliderStyle));
        *($$->slider_style) = $23;
    }
    /* SliderRefColour */
    $$->slider_ref_colour = $24;
}
;
MinValue: eTOK_TAG_MinValue Integer
{
    $$=$2;
    if($$ == 1)
        warn_default("MinValue", @2.first_line);
}
| /* empty */ {$$=1;}
;
InitialValue: eTOK_TAG_InitialValue Integer
{
    $$=snaccAlloc(sizeof(*$$));
    *$$ = $2;
}
| /* empty */ {$$=0;}
;
InitialPortion: eTOK_TAG_InitialPortion Integer
{
    $$=snaccAlloc(sizeof(*$$));
    *$$ = $2;
}
| /* empty */ {$$=0;}
;
StepSize: eTOK_TAG_StepSize Integer
{
    $$=$2;
    if( $$ == 1 )
        warn_default("StepSize", @2.first_line);
}
| /* empty */ {$$=1;}
;
OrientationEnum: eTOK_ENUM_left {$$=LEFT;}
| eTOK_ENUM_right               {$$=RIGHT;}
| eTOK_ENUM_up                  {$$=UP;}
| eTOK_ENUM_down                {$$=DOWN;}
;
SliderStyle: eTOK_TAG_SliderStyle SliderStyleEnum
{
    $$=$2;
    if( $$ == NORMAL )
        warn_default("SliderStyle", @2.first_line);
    
}
| /* empty */ {$$=NORMAL;}
;
SliderStyleEnum: eTOK_ENUM_normal {$$=NORMAL;}
| eTOK_ENUM_thermometer           {$$=THERMOMETER;}
| eTOK_ENUM_proportional          {$$=PROPORTIONAL;}
;
SliderRefColour: eTOK_TAG_SliderRefColour Colour {$$=$2;}
| /* empty */ {$$=0;}
;

/* EntryField Class */
EntryFieldClass: eTOK_LBRACE eTOK_TAG_EntryField
                       ObjectReference
                       InitiallyActive
                       ContentHook
                       OriginalContent
                       Shared
                       eTOK_TAG_OriginalBoxSize
                       Integer
                       Integer
                       OriginalPosition
                       OriginalPaletteRef
                       OriginalFont
                       OptionalFontAttributes
                       OptionalTextColour
                       OptionalBackgroundColour
                       OptionalCharacterSet
                       HorizontalJustification
                       VerticalJustification
                       LineOrientation
                       StartCorner
                       TextWrapping
                       EngineResp
                       HighlightRefColour
                       InputType
                       CharList
                       ObscuredInput
                       MaxLength
                 eTOK_RBRACE 
{
    $$ = snaccAlloc(sizeof(*$$));

    $$->rootClass = $3;
    
    /* InitiallyActive */
    if( $4 == AsnFalse ) {
        $$->initially_active = snaccAlloc(sizeof(AsnBool));
        *($$->initially_active) = AsnFalse;
    }
    
    /* ContentHook */
    if( $5 ) {
        $$->content_hook = $5;
    }
       
    /* OriginalContent */
    $$->original_content = $6;
    if( !$6 ) {
        warn("need OriginalContent for text.", @2.first_line);
    }
    
    /* Shared */
    if( $7 ) {
        $$->shared = snaccAlloc(sizeof(AsnBool));
        *($$->shared) = AsnTrue;
    }

    /* OriginalBoxSize */
    $$->original_box_size = snaccAlloc(sizeof(OriginalBoxSize));
    $$->original_box_size->x_length = $9;
    $$->original_box_size->y_length = $10;

    /* OriginalPosition */
    $$->original_position = $11;

    /* OriginalPaletteRef */
    $$->original_palette_ref = $12;
    /* OriginalFont */
    $$->original_font = $13;
    /* OptionalFontAttributes */
    $$->font_attributes = $14;
    /* OptionalTextColour */
    $$->text_colour = $15;
    /* OptionalBackgroundColour */
    $$->background_colour = $16;
    /* OptionalCharacterSet */
    $$->character_set = $17;
    /* HorizontalJustification */
    if( $18 != START ){
        $$->horizontal_justification = snaccAlloc(sizeof(Justification));
        *($$->horizontal_justification) = $18;
    }
    /* VerticalJustification */
    if( $19 != START ){
        $$->vertical_justification = snaccAlloc(sizeof(Justification));
        *($$->vertical_justification) = $19;
    }
    /* LineOrientation */
    if( $20 != HORIZONTAL ){
        $$->line_orientation = snaccAlloc(sizeof(LineOrientation));
        *($$->line_orientation) = $20;
    }
    /* StartCorner */
    if( $21 != UPPER_LEFT ){
        $$->start_corner = snaccAlloc(sizeof(StartCorner));
        *($$->start_corner) = $21;
    }
    /* TextWrapping */
    if( $22 ){
        $$->text_wrapping = snaccAlloc(sizeof(AsnBool));
        *($$->text_wrapping) = AsnTrue;
    }

    /* EngineResp */
    if( !$23 ){
        $$->engine_resp = snaccAlloc(sizeof(AsnBool));
        *($$->engine_resp) = $23;
    }
    /* HighlightRefColour */
    $$->highlight_ref_colour = $24;

    /* InputType */
    if( $25 != ANY ){
        $$->input_type = snaccAlloc(sizeof(InputType));
        *($$->input_type) = $25;
    }
    /* CharList */
    $$->char_list = $26;
    /* ObscuredInput */
    if( $27 ){
        $$->obscured_input = snaccAlloc(sizeof(AsnBool));
        *($$->obscured_input) = $27;
    } 
    /* MaxLength */
    if( $28 ){
        $$->max_length = snaccAlloc(sizeof(AsnInt));
        *($$->max_length) = $28;
    } 
}
;
InputType: eTOK_TAG_InputType InputTypeEnum
{
    $$=$2;
    if( $$ == ANY )
        warn_default("InputType", @2.first_line);
}
| /* empty */ {$$=ANY;}
;
InputTypeEnum: eTOK_ENUM_alpha {$$ = ALPHA;}
| eTOK_ENUM_numeric            {$$ = NUMERIC;}
| eTOK_ENUM_any                {$$ = ANY;}
| eTOK_ENUM_listed             {$$ = LISTED;}
;
CharList: eTOK_TAG_CharList eTOK_STRING
{
    $$=decode_string($2);
}
| /* empty */
{
    $$.octetLen=0;
    $$.octs = 0;
}
;
ObscuredInput: eTOK_TAG_ObscuredInput eTOK_BOOLEAN
{
    $$=$2;
    if( !$$ )
        warn_default("ObscuredInput", @2.first_line);    
}
| /* empty */ {$$=AsnFalse;}
;
MaxLength: eTOK_TAG_MaxLength Integer
{
    $$=$2;
    if( $$ == 0 )
        warn_default("MaxLength", @2.first_line);    
}
| /* empty */ {$$=0;}
;

/* HyperText Class */
HyperTextClass: eTOK_LBRACE eTOK_TAG_HyperText
                       ObjectReference
                       InitiallyActive
                       ContentHook
                       OriginalContent
                       Shared
                       eTOK_TAG_OriginalBoxSize
                       Integer
                       Integer
                       OriginalPosition
                       OriginalPaletteRef
                       OriginalFont
                       OptionalFontAttributes
                       OptionalTextColour
                       OptionalBackgroundColour
                       OptionalCharacterSet
                       HorizontalJustification
                       VerticalJustification
                       LineOrientation
                       StartCorner
                       TextWrapping
                       EngineResp
                       HighlightRefColour
                 eTOK_RBRACE
{
    $$ = snaccAlloc(sizeof(*$$));

    $$->rootClass = $3;
    
    /* InitiallyActive */
    if( $4 == AsnFalse ) {
        $$->initially_active = snaccAlloc(sizeof(AsnBool));
        *($$->initially_active) = AsnFalse;
    }
    
    /* ContentHook */
    if( $5 ) {
        $$->content_hook = $5;
    }
       
    /* OriginalContent */
    $$->original_content = $6;
    if( !$6 ) {
        warn("need OriginalContent for hypertext.", @2.first_line);
    }
    
    /* Shared */
    if( $7 ) {
        $$->shared = snaccAlloc(sizeof(AsnBool));
        *($$->shared) = AsnTrue;
    }

    /* OriginalBoxSize */
    $$->original_box_size = snaccAlloc(sizeof(OriginalBoxSize));
    $$->original_box_size->x_length = $9;
    $$->original_box_size->y_length = $10;

    /* OriginalPosition */
    $$->original_position = $11;

    /* OriginalPaletteRef */
    $$->original_palette_ref = $12;
    /* OriginalFont */
    $$->original_font = $13;
    /* OptionalFontAttributes */
    $$->font_attributes = $14;
    /* OptionalTextColour */
    $$->text_colour = $15;
    /* OptionalBackgroundColour */
    $$->background_colour = $16;
    /* OptionalCharacterSet */
    $$->character_set = $17;
    /* HorizontalJustification */
    if( $18 != START ){
        $$->horizontal_justification = snaccAlloc(sizeof(Justification));
        *($$->horizontal_justification) = $18;
    }
    /* VerticalJustification */
    if( $19 != START ){
        $$->vertical_justification = snaccAlloc(sizeof(Justification));
        *($$->vertical_justification) = $19;
    }
    /* LineOrientation */
    if( $20 != HORIZONTAL ){
        $$->line_orientation = snaccAlloc(sizeof(LineOrientation));
        *($$->line_orientation) = $20;
    }
    /* StartCorner */
    if( $21 != UPPER_LEFT ){
        $$->start_corner = snaccAlloc(sizeof(StartCorner));
        *($$->start_corner) = $21;
    }
    /* TextWrapping */
    if( $22 ){
        $$->text_wrapping = snaccAlloc(sizeof(AsnBool));
        *($$->text_wrapping) = AsnTrue;
    }

    /* EngineResp */
    if( !$23 ){
        $$->engine_resp = snaccAlloc(sizeof(AsnBool));
        *($$->engine_resp) = $23;
    }
    /* HighlightRefColour */
    $$->highlight_ref_colour = $24;
}
;


/* Button Class */
ButtonRefColour : eTOK_TAG_ButtonRefColour Colour {$$=$2;}
| /* empty. */ {$$=0;}
;


/* Hotspot Class */
HotspotClass: eTOK_LBRACE eTOK_TAG_Hotspot
                  ObjectReference
                  InitiallyActive
                  ContentHook
                  OriginalContent
                  Shared
                  eTOK_TAG_OriginalBoxSize
                  Integer
                  Integer
                  OriginalPosition
                  OriginalPaletteRef
                  EngineResp
                  HighlightRefColour
                  ButtonRefColour
              eTOK_RBRACE
{
    $$ = snaccAlloc(sizeof(*$$));

    $$->rootClass = $3;
    
    /* InitiallyActive */
    if( $4 == AsnFalse ) {
        $$->initially_active = snaccAlloc(sizeof(AsnBool));
        *($$->initially_active) = AsnFalse;
    }
    
    /* ContentHook */
    if( $5 ) 
        warn_bad_field("HotspotClass", "ContentHook", @5.first_line);
       
    /* OriginalContent */
    $$->original_content = $6;
    if( $6 )
        warn_bad_field("HotspotClass", "OriginalContent", @6.first_line);
    
    /* Shared */
    if( $7 ) {
        $$->shared = snaccAlloc(sizeof(AsnBool));
        *($$->shared) = AsnTrue;
    }
       
    /* OriginalBoxSize */
    $$->original_box_size = snaccAlloc(sizeof(OriginalBoxSize));
    $$->original_box_size->x_length = $9;
    $$->original_box_size->y_length = $10;

    /* OriginalPosition */
    $$->original_position = $11;
 
    /* OriginalPaletteRef */
    $$->original_palette_ref = $12;

    /* EngineResp */
    if( !$13 ){
        $$->engine_resp = snaccAlloc(sizeof(AsnBool));
        *($$->engine_resp) = $13;
    }
    /* HighlightRefColour */
    $$->highlight_ref_colour = $14;
    /* ButtonRefColour */
    $$->button_ref_colour = $15;
}
;

/* PushButton Class */
PushButtonClass: eTOK_LBRACE eTOK_TAG_PushButton
                       ObjectReference
                       InitiallyActive
                       ContentHook
                       OriginalContent
                       Shared
                       eTOK_TAG_OriginalBoxSize
                       Integer
                       Integer
                       OriginalPosition
                       OriginalPaletteRef
                       EngineResp
                       HighlightRefColour
                       ButtonRefColour
                       OriginalLabel
                       OptionalCharacterSet
                 eTOK_RBRACE
{
    $$ = snaccAlloc(sizeof(*$$));

    $$->rootClass = $3;
    
    /* InitiallyActive */
    if( $4 == AsnFalse ) {
        $$->initially_active = snaccAlloc(sizeof(AsnBool));
        *($$->initially_active) = AsnFalse;
    }
    
    /* ContentHook */
    if( $5 ) 
        warn_bad_field("PushButton", "ContentHook", @5.first_line);
       
    /* OriginalContent */
    $$->original_content = $6;
    if( $6 )
        warn_bad_field("PushButton", "OriginalContent", @6.first_line);
    
    /* Shared */
    if( $7 ) {
        $$->shared = snaccAlloc(sizeof(AsnBool));
        *($$->shared) = AsnTrue;
    }
       
    /* OriginalBoxSize */
    $$->original_box_size = snaccAlloc(sizeof(OriginalBoxSize));
    $$->original_box_size->x_length = $9;
    $$->original_box_size->y_length = $10;

    /* OriginalPosition */
    $$->original_position = $11;

    /* OriginalPaletteRef */
    $$->original_palette_ref = $12;

    /* EngineResp */
    if( !$13 ){
        $$->engine_resp = snaccAlloc(sizeof(AsnBool));
        *($$->engine_resp) = $13;
    }
    /* HighlightRefColour */
    $$->highlight_ref_colour = $14;
    /* ButtonRefColour */
    $$->button_ref_colour = $15;
    /* OriginalLabel */
    $$->original_label = $16;
    /* OptionalCharacterSet */
    $$->character_set = $17;
}
;
OriginalLabel: eTOK_TAG_OriginalLabel eTOK_STRING
{
    $$=decode_string($2);
}
| /* empty */
{
    $$.octetLen=0;
    $$.octs = 0;
}
;

/* SwitchButton Class */
SwitchButtonClass: eTOK_LBRACE eTOK_TAG_SwitchButton
                       ObjectReference
                       InitiallyActive
                       ContentHook
                       OriginalContent
                       Shared
                       eTOK_TAG_OriginalBoxSize
                       Integer
                       Integer
                       OriginalPosition
                       OriginalPaletteRef
                       EngineResp
                       HighlightRefColour
                       ButtonRefColour
                       OriginalLabel
                       OptionalCharacterSet
                       eTOK_TAG_ButtonStyle ButtonStyleEnum
                   eTOK_RBRACE
{
    $$ = snaccAlloc(sizeof(*$$));

    $$->rootClass = $3;
    
    /* InitiallyActive */
    if( $4 == AsnFalse ) {
        $$->initially_active = snaccAlloc(sizeof(AsnBool));
        *($$->initially_active) = AsnFalse;
    }
    
    /* ContentHook */
    if( $5 ) 
        warn_bad_field("SwitchButton", "ContentHook", @5.first_line);
       
    /* OriginalContent */
    $$->original_content = $6;
    if( $6 )
        warn_bad_field("SwitchButton", "OriginalContent", @6.first_line);
    
    /* Shared */
    if( $7 ) {
        $$->shared = snaccAlloc(sizeof(AsnBool));
        *($$->shared) = AsnTrue;
    }
       
    /* OriginalBoxSize */
    $$->original_box_size = snaccAlloc(sizeof(OriginalBoxSize));
    $$->original_box_size->x_length = $9;
    $$->original_box_size->y_length = $10;

    /* OriginalPosition */
    $$->original_position = $11;

    /* OriginalPaletteRef */
    $$->original_palette_ref = $12;

    /* EngineResp */
    if( !$13 ){
        $$->engine_resp = snaccAlloc(sizeof(AsnBool));
        *($$->engine_resp) = $13;
    }
    /* HighlightRefColour */
    $$->highlight_ref_colour = $14;
    /* ButtonRefColour */
    $$->button_ref_colour = $15;
    /* OriginalLabel */
    $$->original_label = $16;
    /* OptionalCharacterSet */
    $$->character_set = $17;
    /* ButtonStyle */
    $$->button_style = $19;
}
;
ButtonStyleEnum: eTOK_ENUM_pushbutton {$$=PUSHBUTTON;}
| eTOK_ENUM_radiobutton               {$$=RADIOBUTTON;}
| eTOK_ENUM_checkbox                  {$$=CHECKBOX;}
;

/* Action Class */
ActionClass: eTOK_LPAREN ElementaryActionList eTOK_RPAREN {$$=$2;}
;
ElementaryActionList: ElementaryAction ElementaryActionList
{
    /* The prepend will create a node and return pointer to the data.
     */
    *((ElementaryAction **)AsnListPrepend($2)) = $1;
    $$ = $2;
}
| ElementaryAction
{
    /* This should be the last in the list (first reduce) so create the AsnList
     * and the other alternate can prepend to it.
     */
    $$ = AsnListNew(sizeof($1));
    /* The prepend will create a node and return pointer to the data.
     */
    *((ElementaryAction **)AsnListPrepend($$)) = $1;
}
;


ElementaryAction: eTOK_TAG_Activate eTOK_LPAREN GObjRef eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_ACTIVATE;
    $$->a.activate = $3;
}
| eTOK_TAG_Add eTOK_LPAREN GObjRef GInt eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_ADD;
    $$->a.add = snaccAlloc(sizeof(Add));
    $$->a.add->target = $3;
    $$->a.add->value = $4;
}
| eTOK_TAG_AddItem eTOK_LPAREN GObjRef GInt GObjRef eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_ADD_ITEM;
    $$->a.add_item = snaccAlloc(sizeof(AddItem));
    $$->a.add_item->target = $3;
    $$->a.add_item->item_index = $4;
    $$->a.add_item->visible_reference = $5;
}
| eTOK_TAG_Append eTOK_LPAREN GObjRef GOString eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_APPEND;
    $$->a.append = snaccAlloc(sizeof(Append));
    $$->a.append->target = $3;
    $$->a.append->append_value = $4;
}
| eTOK_TAG_BringToFront eTOK_LPAREN GObjRef eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_BRING_TO_FRONT;
    $$->a.bring_to_front = $3;
}
| eTOK_TAG_Call eTOK_LPAREN GObjRef ObjectReference Parameters eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_CALL;
    $$->a.call = snaccAlloc(sizeof(Call));
    $$->a.call->target = $3;
    $$->a.call->call_succeeded = $4;
    $$->a.call->parameters = $5;
}
| eTOK_TAG_CallActionSlot eTOK_LPAREN GObjRef GInt eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_CALL_ACTION_SLOT;
    $$->a.call_action_slot = snaccAlloc(sizeof(CallActionSlot));
    $$->a.call_action_slot->target = $3;
    $$->a.call_action_slot->index = $4;
}
| eTOK_TAG_Clear eTOK_LPAREN GObjRef eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_CLEAR;
    $$->a.clear = $3;
}
| eTOK_TAG_Clone eTOK_LPAREN GObjRef ObjectReference eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_CLONE;
    $$->a.clone = snaccAlloc(sizeof(Clone));
    $$->a.clone->target = $3;
    $$->a.clone->clone_ref_var = $4;
}
| eTOK_TAG_CloseConnection eTOK_LPAREN
                             GObjRef eTOK_TAG_ConnectionTag GInt
                           eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_CLOSE_CONNECTION;
    $$->a.close_connection = snaccAlloc(sizeof(CloseConnection));
    $$->a.close_connection->target = $3;
    $$->a.close_connection->connection_tag = $5;
}
| eTOK_TAG_Deactivate eTOK_LPAREN GObjRef eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_DEACTIVATE;
    $$->a.deactivate = $3;
}
| eTOK_TAG_DelItem eTOK_LPAREN GObjRef GObjRef eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_DEL_ITEM;
    $$->a.del_item = snaccAlloc(sizeof(DelItem));
    $$->a.del_item->target = $3;
    $$->a.del_item->visible_reference = $4;
}
| eTOK_TAG_Deselect eTOK_LPAREN GObjRef eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_DESELECT;
    $$->a.deselect = $3;
}
| eTOK_TAG_DeselectItem eTOK_LPAREN GObjRef GInt eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_DESELECT_ITEM;
    $$->a.deselect_item = snaccAlloc(sizeof(DeselectItem));
    $$->a.deselect_item->target = $3;
    $$->a.deselect_item->item_index = $4;
}
| eTOK_TAG_Divide eTOK_LPAREN GObjRef GInt eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_DIVIDE;
    $$->a.divide = snaccAlloc(sizeof(Divide));
    $$->a.divide->target = $3;
    $$->a.divide->value = $4;
}
| eTOK_TAG_DrawArc eTOK_LPAREN
                      GObjRef GInt GInt GInt GInt GInt GInt
                   eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_DRAW_ARC;
    $$->a.draw_arc = snaccAlloc(sizeof(DrawArc));
    $$->a.draw_arc->target = $3;
    $$->a.draw_arc->x = $4;
    $$->a.draw_arc->y = $5;
    $$->a.draw_arc->ellipse_width = $6;
    $$->a.draw_arc->ellipse_height = $7;
    $$->a.draw_arc->start_angle = $8;
    $$->a.draw_arc->arc_angle = $9;
}
| eTOK_TAG_DrawLine eTOK_LPAREN GObjRef GInt GInt GInt GInt eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_DRAW_LINE;
    $$->a.draw_line = snaccAlloc(sizeof(DrawLine));
    $$->a.draw_line->target = $3;
    $$->a.draw_line->x1 = $4;
    $$->a.draw_line->y1 = $5;
    $$->a.draw_line->x2 = $6;
    $$->a.draw_line->y2 = $7;
}
| eTOK_TAG_DrawOval eTOK_LPAREN GObjRef GInt GInt GInt GInt eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_DRAW_OVAL;
    $$->a.draw_oval = snaccAlloc(sizeof(DrawOval));
    $$->a.draw_oval->target = $3;
    $$->a.draw_oval->x = $4;
    $$->a.draw_oval->y = $5;
    $$->a.draw_oval->ellipse_width = $6;
    $$->a.draw_oval->ellipse_height = $7;
}
| eTOK_TAG_DrawPolygon eTOK_LPAREN GObjRef eTOK_LPAREN PointList eTOK_RPAREN
                       eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_DRAW_POLYGON;
    $$->a.draw_polygon = snaccAlloc(sizeof(DrawPolygon));
    $$->a.draw_polygon->target = $3;
    $$->a.draw_polygon->pointlist = $5;
}
| eTOK_TAG_DrawPolyline eTOK_LPAREN GObjRef eTOK_LPAREN PointList eTOK_RPAREN
                        eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_DRAW_POLYLINE;
    $$->a.draw_polyline = snaccAlloc(sizeof(DrawPolyline));
    $$->a.draw_polyline->target = $3;
    $$->a.draw_polygon->pointlist = $5;
}
| eTOK_TAG_DrawRectangle eTOK_LPAREN GObjRef GInt GInt GInt GInt eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_DRAW_RECTANGLE;
    $$->a.draw_rectangle = snaccAlloc(sizeof(DrawRectangle));
    $$->a.draw_rectangle->target = $3;
    $$->a.draw_rectangle->x1 = $4;
    $$->a.draw_rectangle->y1 = $5;
    $$->a.draw_rectangle->x2 = $6;
    $$->a.draw_rectangle->y2 = $7;
}
| eTOK_TAG_DrawSector eTOK_LPAREN GObjRef GInt GInt GInt GInt GInt GInt
                      eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_DRAW_SECTOR;
    $$->a.draw_sector = snaccAlloc(sizeof(DrawSector));
    $$->a.draw_sector->target = $3;
    $$->a.draw_sector->x = $4;
    $$->a.draw_sector-> y = $5;
    $$->a.draw_sector->ellipse_width = $6;
    $$->a.draw_sector->ellipse_height = $7;
    $$->a.draw_sector->start_angle = $8;
    $$->a.draw_sector->arc_angle = $9;
}
| eTOK_TAG_Fork eTOK_LPAREN GObjRef ObjectReference Parameters eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_FORK;
    $$->a.fork = snaccAlloc(sizeof(Fork));
    $$->a.fork->target = $3;
    $$->a.fork->fork_succeeded = $4;
    $$->a.fork->parameters = $5;
}
| eTOK_TAG_GetAvailabilityStatus eTOK_LPAREN GObjRef ObjectReference
                                 eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_GET_AVAILABILITY_STATUS;
    $$->a.get_availability_status = snaccAlloc(sizeof(GetAvailabilityStatus));
    $$->a.get_availability_status->target = $3;
    $$->a.get_availability_status->availability_status_var = $4;
}
| eTOK_TAG_GetBoxSize eTOK_LPAREN GObjRef ObjectReference ObjectReference
                      eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_GET_BOX_SIZE;
    $$->a.get_box_size = snaccAlloc(sizeof(GetBoxSize));
    $$->a.get_box_size->target = $3;
    $$->a.get_box_size->x_box_size_var = $4;
    $$->a.get_box_size->y_box_size_var = $5;
}
| eTOK_TAG_GetCellItem eTOK_LPAREN GObjRef GInt ObjectReference eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_GET_CELL_ITEM;
    $$->a.get_cell_item = snaccAlloc(sizeof(GetCellItem));
    $$->a.get_cell_item->target = $3;
    $$->a.get_cell_item->cell_index = $4;
    $$->a.get_cell_item->item_ref_var = $5;
}
| eTOK_TAG_GetCursorPosition eTOK_LPAREN
                                 GObjRef ObjectReference ObjectReference
                             eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_GET_CURSOR_POSITION;
    $$->a.get_cursor_position = snaccAlloc(sizeof(GetCursorPosition));
    $$->a.get_cursor_position->target = $3;
    $$->a.get_cursor_position->x_out = $4;
    $$->a.get_cursor_position->y_out = $5;
}
| eTOK_TAG_GetEngineSupport eTOK_LPAREN
                                 GObjRef GOString ObjectReference
                            eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_GET_ENGINE_SUPPORT;
    $$->a.get_engine_support = snaccAlloc(sizeof(GetEngineSupport));
    $$->a.get_engine_support->target = $3;
    $$->a.get_engine_support->feature = $4;
    $$->a.get_engine_support->answer = $5;
}
| eTOK_TAG_GetEntryPoint eTOK_LPAREN GObjRef ObjectReference eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_GET_ENTRY_POINT;
    $$->a.get_entry_point = snaccAlloc(sizeof(GetEntryPoint));
    $$->a.get_entry_point->target = $3;
    $$->a.get_entry_point->entry_point_var = $4;
}
| eTOK_TAG_GetFillColour eTOK_LPAREN GObjRef ObjectReference eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_GET_FILL_COLOUR;
    $$->a.get_fill_colour = snaccAlloc(sizeof(GetFillColour));
    $$->a.get_fill_colour->target = $3;
    $$->a.get_fill_colour->fill_colour_var = $4;
}
| eTOK_TAG_GetFirstItem eTOK_LPAREN GObjRef ObjectReference eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_GET_FIRST_ITEM;
    $$->a.get_first_item = snaccAlloc(sizeof(GetFirstItem));
    $$->a.get_first_item->target = $3;
    $$->a.get_first_item->first_item_var = $4;
}
| eTOK_TAG_GetHighlightStatus eTOK_LPAREN GObjRef ObjectReference eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_GET_HIGHLIGHT_STATUS;
    $$->a.get_highlight_status = snaccAlloc(sizeof(GetHighlightStatus));
    $$->a.get_highlight_status->target = $3;
    $$->a.get_highlight_status->highlight_status_var = $4;
}
| eTOK_TAG_GetInteractionStatus eTOK_LPAREN GObjRef ObjectReference eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_GET_INTERACTION_STATUS;
    $$->a.get_interaction_status = snaccAlloc(sizeof(GetInteractionStatus));
    $$->a.get_interaction_status->target = $3;
    $$->a.get_interaction_status->interaction_status_var = $4;
}
| eTOK_TAG_GetItemStatus eTOK_LPAREN GObjRef GInt ObjectReference eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_GET_ITEM_STATUS;
    $$->a.get_item_status = snaccAlloc(sizeof(GetItemStatus));
    $$->a.get_item_status->target = $3;
    $$->a.get_item_status->item_index = $4;
    $$->a.get_item_status->item_status_var = $5;
}
| eTOK_TAG_GetLabel eTOK_LPAREN GObjRef ObjectReference eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_GET_LABEL;
    $$->a.get_label = snaccAlloc(sizeof(GetLabel));
    $$->a.get_label->target = $3;
    $$->a.get_label->label_var = $4;
}
| eTOK_TAG_GetLastAnchorFired eTOK_LPAREN GObjRef ObjectReference eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_GET_LAST_ANCHOR_FIRED;
    $$->a.get_last_anchor_fired = snaccAlloc(sizeof(GetLastAnchorFired));
    $$->a.get_last_anchor_fired->target = $3;
    $$->a.get_last_anchor_fired->last_anchor_fired_var = $4;
}
| eTOK_TAG_GetLineColour eTOK_LPAREN GObjRef ObjectReference eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_GET_LINE_COLOUR;
    $$->a.get_line_colour = snaccAlloc(sizeof(GetLineColour));
    $$->a.get_line_colour->target = $3;
    $$->a.get_line_colour->line_colour_var = $4;
}
| eTOK_TAG_GetLineStyle eTOK_LPAREN GObjRef ObjectReference eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_GET_LINE_STYLE;
    $$->a.get_line_style = snaccAlloc(sizeof(GetLineStyle));
    $$->a.get_line_style->target = $3;
    $$->a.get_line_style->line_style_var = $4;
}
| eTOK_TAG_GetLineWidth eTOK_LPAREN GObjRef ObjectReference eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_GET_LINE_WIDTH;
    $$->a.get_line_width = snaccAlloc(sizeof(GetLineWidth));
    $$->a.get_line_width->target = $3;
    $$->a.get_line_width->line_width_var = $4;
}
| eTOK_TAG_GetListItem eTOK_LPAREN GObjRef GInt ObjectReference eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_GET_LIST_ITEM;
    $$->a.get_list_item = snaccAlloc(sizeof(GetListItem));
    $$->a.get_list_item->target = $3;
    $$->a.get_list_item->item_index = $4;
    $$->a.get_list_item->item_ref_var = $5;
}
| eTOK_TAG_GetListSize eTOK_LPAREN GObjRef ObjectReference eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_GET_LIST_SIZE;
    $$->a.get_list_size = snaccAlloc(sizeof(GetListSize));
    $$->a.get_list_size->target = $3;
    $$->a.get_list_size->size_var = $4;
}
| eTOK_TAG_GetOverwriteMode eTOK_LPAREN GObjRef ObjectReference eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_GET_OVERWRITE_MODE;
    $$->a.get_overwrite_mode = snaccAlloc(sizeof(GetOverwriteMode));
    $$->a.get_overwrite_mode->target = $3;
    $$->a.get_overwrite_mode->overwrite_mode_var = $4;
}
| eTOK_TAG_GetPortion eTOK_LPAREN GObjRef ObjectReference eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_GET_PORTION;
    $$->a.get_portion = snaccAlloc(sizeof(GetPortion));
    $$->a.get_portion->target = $3;
    $$->a.get_portion->portion_var = $4;
}
| eTOK_TAG_GetPosition eTOK_LPAREN GObjRef ObjectReference ObjectReference
                       eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_GET_POSITION;
    $$->a.get_position = snaccAlloc(sizeof(GetPosition));
    $$->a.get_position->target = $3;
    $$->a.get_position->x_position_var = $4;
    $$->a.get_position->y_position_var = $5;
}
| eTOK_TAG_GetRunningStatus eTOK_LPAREN GObjRef ObjectReference eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_GET_RUNNING_STATUS;
    $$->a.get_running_status = snaccAlloc(sizeof(GetRunningStatus));
    $$->a.get_running_status->target = $3;
    $$->a.get_running_status->running_status_var = $4;
}
| eTOK_TAG_GetSelectionStatus eTOK_LPAREN GObjRef ObjectReference eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_GET_SELECTION_STATUS;
    $$->a.get_selection_status = snaccAlloc(sizeof(GetSelectionStatus));
    $$->a.get_selection_status->target = $3;
    $$->a.get_selection_status->selection_status_var = $4;
}
| eTOK_TAG_GetSliderValue eTOK_LPAREN GObjRef ObjectReference  eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_GET_SLIDER_VALUE;
    $$->a.get_slider_value = snaccAlloc(sizeof(GetSliderValue));
    $$->a.get_slider_value->target = $3;
    $$->a.get_slider_value->slider_value_var = $4;
}
| eTOK_TAG_GetTextContent eTOK_LPAREN GObjRef ObjectReference eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_GET_TEXT_CONTENT;
    $$->a.get_text_content = snaccAlloc(sizeof(GetTextContent));
    $$->a.get_text_content->target = $3;
    $$->a.get_text_content->text_content_var = $4;
}
| eTOK_TAG_GetTextData eTOK_LPAREN GObjRef ObjectReference eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_GET_TEXT_DATA;
    $$->a.get_text_data = snaccAlloc(sizeof(GetTextData));
    $$->a.get_text_data->target = $3;
    $$->a.get_text_data->text_data_var = $4;
}
| eTOK_TAG_GetTokenPosition eTOK_LPAREN GObjRef ObjectReference eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_GET_TOKEN_POSITION;
    $$->a.get_token_position = snaccAlloc(sizeof(GetTokenPosition));
    $$->a.get_token_position->target = $3;
    $$->a.get_token_position->token_position_var = $4;
}
| eTOK_TAG_GetVolume eTOK_LPAREN GObjRef ObjectReference eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_GET_VOLUME;
    $$->a.get_volume = snaccAlloc(sizeof(GetVolume));
    $$->a.get_volume->target = $3;
    $$->a.get_volume->volume_var = $4;
}
| eTOK_TAG_Launch eTOK_LPAREN GObjRef eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_LAUNCH;
    $$->a.launch = $3;
}
| eTOK_TAG_LockScreen eTOK_LPAREN GObjRef eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_LOCK_SCREEN;
    $$->a.lock_screen = $3;
}
| eTOK_TAG_Modulo eTOK_LPAREN GObjRef GInt eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_MODULO;
    $$->a.modulo = snaccAlloc(sizeof(Modulo));
    $$->a.modulo->target = $3;
    $$->a.modulo->value = $4;
}
| eTOK_TAG_Move eTOK_LPAREN GObjRef GInt eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_MOVE;
    $$->a.move = snaccAlloc(sizeof(Move));
    $$->a.move->target = $3;
    $$->a.move->movement_identifier = $4;
}
| eTOK_TAG_MoveTo eTOK_LPAREN GObjRef GInt eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_MOVE_TO;
    $$->a.move_to = snaccAlloc(sizeof(MoveTo));
    $$->a.move_to->target = $3;
    $$->a.move_to->index = $4;
}
| eTOK_TAG_Multiply eTOK_LPAREN GObjRef GInt eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_MULTIPLY;
    $$->a.multiply = snaccAlloc(sizeof(Multiply));
    $$->a.multiply->target = $3;
    $$->a.multiply->value = $4;
}
| eTOK_TAG_OpenConnection eTOK_LPAREN
                             GObjRef
                             ObjectReference
                             GOString
                             GOString
                             eTOK_TAG_ConnectionTag
                             GInt
                          eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_OPEN_CONNECTION;
    $$->a.open_connection = snaccAlloc(sizeof(OpenConnection));
    $$->a.open_connection->target = $3;
    $$->a.open_connection->open_succeeded = $4;
    $$->a.open_connection->protocol = $5;
    $$->a.open_connection->address = $6;
    $$->a.open_connection->connection_tag = $8;
}
| eTOK_TAG_Preload eTOK_LPAREN GObjRef eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_PRELOAD;
    $$->a.preload = $3;
}
| eTOK_TAG_PutBefore eTOK_LPAREN GObjRef GObjRef eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_PUT_BEFORE;
    $$->a.put_before = snaccAlloc(sizeof(PutBefore));
    $$->a.put_before->target = $3;
    $$->a.put_before->reference_visible = $4;
}
| eTOK_TAG_PutBehind eTOK_LPAREN GObjRef GObjRef eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_PUT_BEHIND;
    $$->a.put_behind = snaccAlloc(sizeof(PutBehind));
    $$->a.put_behind->target = $3;
    $$->a.put_behind->reference_visible = $4;
}
| eTOK_TAG_Quit eTOK_LPAREN GObjRef eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_QUIT;
    $$->a.quit = $3;
}
| eTOK_TAG_ReadPersistent eTOK_LPAREN
                               GObjRef ObjectReference eTOK_LPAREN
                                                        VariableList
                                                       eTOK_RPAREN
                               GOString
                          eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_READ_PERSISTENT;
    $$->a.read_persistent = snaccAlloc(sizeof(ReadPersistent));
    $$->a.read_persistent->target = $3;
    $$->a.read_persistent->read_succeeded = $4;
    $$->a.read_persistent->out_variables = $6;
    $$->a.read_persistent->in_file_name = $8;
}
| eTOK_TAG_Run eTOK_LPAREN GObjRef eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_RUN;
    $$->a.run = $3;
}
| eTOK_TAG_ScaleBitmap eTOK_LPAREN GObjRef GInt GInt eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_SCALE_BITMAP;
    $$->a.scale_bitmap = snaccAlloc(sizeof(ScaleBitmap));
    $$->a.scale_bitmap->target = $3;
    $$->a.scale_bitmap->x_scale = $4;
    $$->a.scale_bitmap->y_scale = $5;
}
| eTOK_TAG_ScaleVideo eTOK_LPAREN GObjRef GInt GInt eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_SCALE_VIDEO;
    $$->a.scale_video = snaccAlloc(sizeof(ScaleVideo));
    $$->a.scale_video->target = $3;
    $$->a.scale_video->x_scale = $4;
    $$->a.scale_video->y_scale = $5;  
}
| eTOK_TAG_ScrollItems eTOK_LPAREN GObjRef GInt eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_SCROLL_ITEMS;
    $$->a.scroll_items = snaccAlloc(sizeof(ScrollItems));
    $$->a.scroll_items->target = $3;
    $$->a.scroll_items->items_to_scroll = $4;
}
| eTOK_TAG_Select eTOK_LPAREN GObjRef eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_SELECT;
    $$->a.select = $3;
}
| eTOK_TAG_SelectItem eTOK_LPAREN GObjRef GInt eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_SELECT_ITEM;
    $$->a.select_item = snaccAlloc(sizeof(SelectItem));
    $$->a.select_item->target = $3;
    $$->a.select_item->item_index = $4;
}
| eTOK_TAG_SendEvent eTOK_LPAREN
                          GObjRef GObjRef EventTypeEnum EmulatedEventData
                     eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_SEND_EVENT;
    $$->a.send_event = snaccAlloc(sizeof(SendEvent));
    $$->a.send_event->target = $3;
    $$->a.send_event->emulated_event_source = $4;
    $$->a.send_event->emulated_event_type = $5;
    $$->a.send_event->emulated_event_data = $6;
}
| eTOK_TAG_SendToBack eTOK_LPAREN GObjRef eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_SEND_TO_BACK;
    $$->a.send_to_back = $3;
}
| eTOK_TAG_SetBoxSize eTOK_LPAREN GObjRef GInt GInt eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_SET_BOX_SIZE;
    $$->a.set_box_size = snaccAlloc(sizeof(SetBoxSize));
    $$->a.set_box_size->target = $3;
    $$->a.set_box_size->x_new_box_size = $4;
    $$->a.set_box_size->y_new_box_size = $5;
}
| eTOK_TAG_SetCachePriority eTOK_LPAREN GObjRef GInt eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_SET_CACHE_PRIORITY;
    $$->a.set_cache_priority = snaccAlloc(sizeof(SetCachePriority));
    $$->a.set_cache_priority->target = $3;
    $$->a.set_cache_priority->new_cache_priority = $4;
}
| eTOK_TAG_SetCounterEndPosition eTOK_LPAREN GObjRef GInt eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_SET_COUNTER_END_POSITION;
    $$->a.set_counter_end_position = snaccAlloc(sizeof(SetCounterEndPosition));
    $$->a.set_counter_end_position->target = $3;
    $$->a.set_counter_end_position->new_counter_end_position = $4;
}
| eTOK_TAG_SetCounterPosition eTOK_LPAREN GObjRef GInt eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_SET_COUNTER_POSITION;
    $$->a.set_counter_position = snaccAlloc(sizeof(SetCounterPosition));
    $$->a.set_counter_position->target = $3;
    $$->a.set_counter_position->new_counter_position = $4;
}
| eTOK_TAG_SetCounterTrigger eTOK_LPAREN GObjRef GInt NewCounterValue
                             eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_SET_COUNTER_TRIGGER;
    $$->a.set_counter_trigger = snaccAlloc(sizeof(SetCounterTrigger));
    $$->a.set_counter_trigger->target = $3;
    $$->a.set_counter_trigger->trigger_identifier = $4;
    $$->a.set_counter_trigger->new_counter_value = $5;
}
| eTOK_TAG_SetCursorPosition eTOK_LPAREN GObjRef GInt GInt eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_SET_CURSOR_POSITION;
    $$->a.set_cursor_position = snaccAlloc(sizeof(SetCursorPosition));
    $$->a.set_cursor_position->target = $3;
    $$->a.set_cursor_position->x_cursor = $4;
    $$->a.set_cursor_position->y_cursor = $5;
}
| eTOK_TAG_SetCursorShape eTOK_LPAREN GObjRef NewCursorShape eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_SET_CURSOR_SHAPE;
    $$->a.set_cursor_shape = snaccAlloc(sizeof(SetCursorShape));
    $$->a.set_cursor_shape->target = $3;
    $$->a.set_cursor_shape->new_cursor_shape = $4;
}
| eTOK_TAG_SetData eTOK_LPAREN GObjRef NewContent eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_SET_DATA;
    $$->a.set_data = snaccAlloc(sizeof(SetData));
    $$->a.set_data->target = $3;
    $$->a.set_data->new_content = $4;
}
| eTOK_TAG_SetEntryPoint eTOK_LPAREN GObjRef GInt eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_SET_ENTRY_POINT;
    $$->a.set_entry_point = snaccAlloc(sizeof(SetEntryPoint));
    $$->a.set_entry_point->target = $3;
    $$->a.set_entry_point->new_entry_point = $4;
}
| eTOK_TAG_SetFillColour eTOK_LPAREN GObjRef NewColourOrNone eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_SET_FILL_COLOUR;
    $$->a.set_fill_colour = snaccAlloc(sizeof(SetFillColour));
    $$->a.set_fill_colour->target = $3;
    $$->a.set_fill_colour->new_fill_colour = $4;
}
| eTOK_TAG_SetFirstItem eTOK_LPAREN GObjRef GInt eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_SET_FIRST_ITEM;
    $$->a.set_first_item = snaccAlloc(sizeof(SetFirstItem));
    $$->a.set_first_item->target = $3;
    $$->a.set_first_item->new_first_item = $4;
}
| eTOK_TAG_SetFontRef eTOK_LPAREN GObjRef NewFont eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_SET_FONT_REF;
    $$->a.set_font_ref = snaccAlloc(sizeof(SetFontRef));
    $$->a.set_font_ref->target = $3;
    $$->a.set_font_ref->new_font = $4;
}
| eTOK_TAG_SetHighlightStatus eTOK_LPAREN GObjRef GBool eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_SET_HIGHLIGHT_STATUS;
    $$->a.set_highlight_status = snaccAlloc(sizeof(SetHighlightStatus));
    $$->a.set_highlight_status->target = $3;
    $$->a.set_highlight_status->new_highlight_status = $4;
}
| eTOK_TAG_SetInteractionStatus eTOK_LPAREN GObjRef GBool eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_SET_INTERACTION_STATUS;
    $$->a.set_interaction_status = snaccAlloc(sizeof(SetInteractionStatus));
    $$->a.set_interaction_status->target = $3;
    $$->a.set_interaction_status->new_interaction_status = $4;
}
| eTOK_TAG_SetLabel eTOK_LPAREN GObjRef GOString eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_SET_LABEL;
    $$->a.set_label = snaccAlloc(sizeof(SetLabel));
    $$->a.set_label->target = $3;
    $$->a.set_label->new_label = $4;
}
| eTOK_TAG_SetLineColour eTOK_LPAREN GObjRef NewColour eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_SET_LINE_COLOUR;
    $$->a.set_line_colour = snaccAlloc(sizeof(SetLineColour));
    $$->a.set_line_colour->target = $3;
    $$->a.set_line_colour->new_line_colour = $4;
}
| eTOK_TAG_SetLineStyle eTOK_LPAREN GObjRef GInt eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_SET_LINE_STYLE;
    $$->a.set_line_style = snaccAlloc(sizeof(SetLineStyle));
    $$->a.set_line_style->target = $3;
    $$->a.set_line_style->new_line_style = $4;
}
| eTOK_TAG_SetLineWidth eTOK_LPAREN GObjRef GInt eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_SET_LINE_WIDTH;
    $$->a.set_line_width = snaccAlloc(sizeof(SetLineWidth));
    $$->a.set_line_width->target = $3;
    $$->a.set_line_width->new_line_width = $4;
}
| eTOK_TAG_SetOverwriteMode eTOK_LPAREN GObjRef GBool eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_SET_OVERWRITE_MODE;
    $$->a.set_overwrite_mode = snaccAlloc(sizeof(SetOverwriteMode));
    $$->a.set_overwrite_mode->target = $3;
    $$->a.set_overwrite_mode->new_overwrite_mode = $4;
}
| eTOK_TAG_SetPaletteRef eTOK_LPAREN GObjRef GObjRef eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_SET_PALETTE_REF;
    $$->a.set_palette_ref = snaccAlloc(sizeof(SetPaletteRef));
    $$->a.set_palette_ref->target = $3;
    $$->a.set_palette_ref->new_palette_ref = $4;
}
| eTOK_TAG_SetPortion eTOK_LPAREN GObjRef GInt eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_SET_PORTION;
    $$->a.set_portion = snaccAlloc(sizeof(SetPortion));
    $$->a.set_portion->target = $3;
    $$->a.set_portion->new_portion = $4;
}
| eTOK_TAG_SetPosition eTOK_LPAREN GObjRef GInt GInt eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_SET_POSITION;
    $$->a.set_position = snaccAlloc(sizeof(SetPosition));
    $$->a.set_position->target = $3;
    $$->a.set_position->new_x_position = $4;
    $$->a.set_position->new_y_position = $5;
}
| eTOK_TAG_SetSliderValue eTOK_LPAREN GObjRef GInt eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_SET_SLIDER_VALUE;
    $$->a.set_slider_value = snaccAlloc(sizeof(SetSliderValue));
    $$->a.set_slider_value->target = $3;
    $$->a.set_slider_value->new_slider_value = $4;
}
| eTOK_TAG_SetSpeed eTOK_LPAREN GObjRef Rational eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_SET_SPEED;
    $$->a.set_speed = snaccAlloc(sizeof(SetSpeed));
    $$->a.set_speed->target = $3;
    $$->a.set_speed->new_speed = $4;
}
| eTOK_TAG_SetTimer eTOK_LPAREN GObjRef GInt TimerValue AbsoluteTime
                    eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_SET_TIMER;
    $$->a.set_timer = snaccAlloc(sizeof(SetTimer));
    $$->a.set_timer->target = $3;
    $$->a.set_timer->timer_id = $4;
    if( $5 ){        
        $$->a.set_timer->new_timer = snaccAlloc(sizeof(NewTimer));
        $$->a.set_timer->new_timer->timer_value = $5;
        $$->a.set_timer->new_timer->absolute_time = $6;
    }
}
| eTOK_TAG_SetTransparency eTOK_LPAREN GObjRef GInt eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_SET_TRANSPARENCY;
    $$->a.set_transparency = snaccAlloc(sizeof(SetTransparency));
    $$->a.set_transparency->target = $3;
    $$->a.set_transparency->new_transparency = $4;
}
| eTOK_TAG_SetVariable eTOK_LPAREN GObjRef NewVariableValue eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_SET_VARIABLE;
    $$->a.set_variable = snaccAlloc(sizeof(SetVariable));
    $$->a.set_variable->target = $3;
    $$->a.set_variable->new_variable_value = $4;
}
| eTOK_TAG_SetVolume eTOK_LPAREN GObjRef GInt eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_SET_VOLUME;
    $$->a.set_volume = snaccAlloc(sizeof(SetVolume));
    $$->a.set_volume->target = $3;
    $$->a.set_volume->new_volume = $4;
}
| eTOK_TAG_Spawn eTOK_LPAREN GObjRef eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_SPAWN;
    $$->a.spawn = $3;
}
| eTOK_TAG_Step eTOK_LPAREN GObjRef GInt eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_STEP;
    $$->a.step = snaccAlloc(sizeof(Step));
    $$->a.step->target = $3;
    $$->a.step->nb_of_steps = $4;
}
| eTOK_TAG_Stop eTOK_LPAREN GObjRef eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_STOP;
    $$->a.stop = $3;
}

| eTOK_TAG_StorePersistent eTOK_LPAREN
                             GObjRef ObjectReference  eTOK_LPAREN
                                                        VariableList
                                                       eTOK_RPAREN
                               GOString
                           eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_STORE_PERSISTENT;
    $$->a.store_persistent = snaccAlloc(sizeof(StorePersistent));
    $$->a.store_persistent->target = $3;
    $$->a.store_persistent->store_succeeded = $4;
    $$->a.store_persistent->in_variables = $6;
    $$->a.store_persistent->out_file_name = $8;
}
| eTOK_TAG_Subtract eTOK_LPAREN GObjRef GInt eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_SUBTRACT;
    $$->a.subtract = snaccAlloc(sizeof(Subtract));
    $$->a.subtract->target = $3;
    $$->a.subtract->value = $4;
}
| eTOK_TAG_TestVariable eTOK_LPAREN
                           GObjRef ComparisonOperator ComparisonValue
                        eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_TEST_VARIABLE;
    $$->a.test_variable = snaccAlloc(sizeof(TestVariable));
    $$->a.test_variable->target = $3;
    $$->a.test_variable->operator = $4;
    $$->a.test_variable->comparison_value = $5;
}
| eTOK_TAG_Toggle eTOK_LPAREN GObjRef eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_TOGGLE;
    $$->a.toggle = $3;
}
| eTOK_TAG_ToggleItem eTOK_LPAREN GObjRef GInt eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_TOGGLE_ITEM;
    $$->a.toggle_item = snaccAlloc(sizeof(ToggleItem));
    $$->a.toggle_item->target = $3;
    $$->a.toggle_item->item_index = $4;
}
| eTOK_TAG_TransitionTo eTOK_LPAREN GObjRef ConnectionTag TransitionEffect
                        eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_TRANSITION_TO;
    $$->a.transition_to = snaccAlloc(sizeof(TransitionTo));
    $$->a.transition_to->target = $3;
    $$->a.transition_to->connection_tag_or_null
        = snaccAlloc(sizeof(ConnectionTagOrNull));
    if( $4 ){
        $$->a.transition_to->connection_tag_or_null->choiceId
            = CONNECTIONTAGORNULL_CONNECTION_TAG;
        $$->a.transition_to->connection_tag_or_null->a.connection_tag = $4;
    }
    else {
        $$->a.transition_to->connection_tag_or_null->choiceId
            = CONNECTIONTAGORNULL_NULL;
    }
    $$->a.transition_to->transition_effect = $5;
}
| eTOK_TAG_Unload eTOK_LPAREN GObjRef eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_UNLOAD;
    $$->a.unload = $3;
}
| eTOK_TAG_UnlockScreen eTOK_LPAREN GObjRef eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_UNLOCK_SCREEN;
    $$->a.unlock_screen = $3;
}
| eTOK_TAG_SetBackgroundColour eTOK_LPAREN GObjRef NewColour eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_SET_BACKGROUND_COLOUR;
    $$->a.set_background_colour = snaccAlloc(sizeof(SetBackgroundColour));
    $$->a.set_background_colour->target = $3;
    $$->a.set_background_colour->new_background_colour = $4;
}
| eTOK_TAG_SetTextColour eTOK_LPAREN GObjRef NewColour eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_SET_TEXT_COLOUR;
    $$->a.set_text_colour = snaccAlloc(sizeof(SetTextColour));
    $$->a.set_text_colour->target = $3;
    $$->a.set_text_colour->new_text_colour = $4;
}
| eTOK_TAG_SetFontAttributes eTOK_LPAREN GObjRef GOString eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_SET_FONT_ATTRIBUTES;
    $$->a.set_font_attributes = snaccAlloc(sizeof(SetFontAttributes));
    $$->a.set_font_attributes->target = $3;
    $$->a.set_font_attributes->new_font_attributes = $4;    
}
| eTOK_TAG_SetCellPosition eTOK_LPAREN GObjRef GInt GInt GInt eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_SET_CELL_POSITION;
    $$->a.set_cell_position = snaccAlloc(sizeof(SetCellPosition));
    $$->a.set_cell_position->target = $3;
    $$->a.set_cell_position->index = $4;
    $$->a.set_cell_position->new_x_position = $5;
    $$->a.set_cell_position->new_y_position = $6;
}
| eTOK_TAG_SetInputRegister eTOK_LPAREN GObjRef GInt eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_SET_INPUT_REGISTER;
    $$->a.set_input_register = snaccAlloc(sizeof(SetInputRegister));
    $$->a.set_input_register->target = $3;
    $$->a.set_input_register->new_input_register = $4;
}
| eTOK_TAG_SetVideoDecodeOffset eTOK_LPAREN GObjRef GInt GInt eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_SET_VIDEO_DECODE_OFFSET;
    $$->a.set_position_offset = snaccAlloc(sizeof(SetPositionOffset));
    $$->a.set_position_offset->target = $3;
    $$->a.set_position_offset->new_x_offset = $4;
    $$->a.set_position_offset->new_y_offset = $5;
}
| eTOK_TAG_GetVideoDecodeOffset eTOK_LPAREN
                                 GObjRef ObjectReference ObjectReference
                             eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_GET_VIDEO_DECODE_OFFSET;
    $$->a.get_position_offset = snaccAlloc(sizeof(GetPositionOffset));
    $$->a.get_position_offset->target = $3;
    $$->a.get_position_offset->x_offset_var = $4;
    $$->a.get_position_offset->y_offset_var = $5;
}
| eTOK_TAG_SetFocusPosition eTOK_LPAREN GObjRef GInt eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_SET_FOCUS_POSITION;
    $$->a.set_focus_position = snaccAlloc(sizeof(SetFocusPosition));
    $$->a.set_focus_position->target = $3;
    $$->a.set_focus_position->new_focus_position = $4;
}
| eTOK_TAG_GetFocusPosition eTOK_LPAREN GObjRef ObjectReference eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_GET_FOCUS_POSITION;
    $$->a.get_focus_position = snaccAlloc(sizeof(GetFocusPosition));
    $$->a.get_focus_position->target = $3;
    $$->a.get_focus_position->focus_position_var = $4;
}
| eTOK_TAG_SetBitmapDecodeOffset eTOK_LPAREN GObjRef GInt GInt eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_SET_BITMAP_DECODE_OFFSET;
    $$->a.set_position_offset = snaccAlloc(sizeof(SetPositionOffset));
    $$->a.set_position_offset->target = $3;
    $$->a.set_position_offset->new_x_offset = $4;
    $$->a.set_position_offset->new_y_offset = $5;
}
| eTOK_TAG_GetBitmapDecodeOffset eTOK_LPAREN
                                 GObjRef ObjectReference ObjectReference
                             eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_GET_BITMAP_DECODE_OFFSET;
    $$->a.get_position_offset = snaccAlloc(sizeof(GetPositionOffset));
    $$->a.get_position_offset->target = $3;
    $$->a.get_position_offset->x_offset_var = $4;
    $$->a.get_position_offset->y_offset_var = $5;
}
| eTOK_TAG_SetSliderParameters eTOK_LPAREN GObjRef GInt GInt GInt eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_SET_SLIDER_PARAMETERS;
    $$->a.set_slider_parameters = snaccAlloc(sizeof(SetSliderParameters));
    $$->a.set_slider_parameters->target = $3;
    $$->a.set_slider_parameters->new_min_value = $4;
    $$->a.set_slider_parameters->new_max_value = $5;
    $$->a.set_slider_parameters->new_step_size = $6;
}
| eTOK_TAG_SetDesktopColour eTOK_LPAREN GObjRef NewColour eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = ELEMENTARYACTION_SET_DESKTOP_COLOUR;
    $$->a.set_desktop_colour = snaccAlloc(sizeof(SetDesktopColour));
    $$->a.set_desktop_colour->target = $3;
    $$->a.set_desktop_colour->new_desktop_colour = $4;
}
;
        
/***********************************************/

Parameters: ParameterList {$$=$1;}
| /* empty. */ {$$=0;}
;
ParameterList: Parameter ParameterList
{
    /* The prepend will create a node and return pointer to the data.
     */
    *((Parameter **)AsnListPrepend($2)) = $1;
    $$ = $2;
}
| Parameter
{
    /* This should be the last in the list (first reduce) so create the AsnList
     * and the other alternate can prepend to it.
     */
    $$ = AsnListNew(sizeof($1));
    /* The prepend will create a node and return pointer to the data.
     */
    *((Parameter **)AsnListPrepend($$)) = $1;
}
;
Parameter: eTOK_TAG_GenericBoolean GBool
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = PARAMETER_NEW_GENERIC_BOOLEAN;
    $$->a.new_generic_boolean = $2;
}
| eTOK_TAG_GenericInteger GInt
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = PARAMETER_NEW_GENERIC_INTEGER;
    $$->a.new_generic_integer = $2;
}
| eTOK_TAG_GenericOctetString GOString
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = PARAMETER_NEW_GENERIC_OCTETSTRING;
    $$->a.new_generic_octetstring = $2;
}
| eTOK_TAG_GenericObjectReference GObjRef
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = PARAMETER_NEW_GENERIC_OBJECT_REFERENCE;
    $$->a.new_generic_object_reference = $2;
}
| eTOK_TAG_GenericContentReference GContRef
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = PARAMETER_NEW_GENERIC_CONTENT_REFERENCE;
    $$->a.new_generic_content_reference = $2;
}
;
PointList: Point PointList {$$=0;}
{
    /* The prepend will create a node and return pointer to the data.
     */
    *((Point **)AsnListPrepend($2)) = $1;
    $$ = $2;
}
| Point
{
    /* This should be the last in the list (first reduce) so create the AsnList
     * and the other alternate can prepend to it.
     */
    $$ = AsnListNew(sizeof($1));
    /* The prepend will create a node and return pointer to the data.
     */
    *((Point **)AsnListPrepend($$)) = $1;
}
;
Point: eTOK_LPAREN GInt GInt eTOK_RPAREN
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->x = $2;
    $$->y = $3;
}
;
VariableList: ObjectReference VariableList
{
    /* The prepend will create a node and return pointer to the data.
     */
    *((ObjectReference **)AsnListPrepend($2)) = $1;
    $$ = $2;
}
| ObjectReference
{
    /* This should be the last in the list (first reduce) so create the AsnList
     * and the other alternate can prepend to it.
     */
    $$ = AsnListNew(sizeof(*$1));
    /* The prepend will create a node and return pointer to the data.
     */
    *((ObjectReference **)AsnListPrepend($$)) = $1;
}
;

EmulatedEventData: eTOK_TAG_GenericBoolean GBool
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = EMULATEDEVENTDATA_NEW_GENERIC_BOOLEAN;
    $$->a.new_generic_boolean = $2;
}
| eTOK_TAG_GenericInteger GInt
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = EMULATEDEVENTDATA_NEW_GENERIC_INTEGER;
    $$->a.new_generic_integer = $2;
}
| eTOK_TAG_GenericOctetString GOString 
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = EMULATEDEVENTDATA_NEW_GENERIC_OCTET_STRING;
    $$->a.new_generic_octet_string = $2;
}
| /* empty */ {$$=0;}
;
NewCounterValue : GInt {$$=$1;}
| /* empty */ {$$=0;}
;
NewCursorShape: GObjRef {$$=$1;}
| {$$=0;}
;
NewContent: GOString
{
    $$=snaccAlloc(sizeof(*$$));
    $$->choiceId = NEWCONTENT_NEW_INCLUDED_CONTENT;
    $$->a.new_included_content = $1;
}
| eTOK_TAG_NewReferencedContent eTOK_LPAREN
                                  GContRef
                                  NewContentSize
                                  NewContentCachePriority
                                eTOK_RPAREN
{
    $$=snaccAlloc(sizeof(*$$));
    $$->choiceId = NEWCONTENT_NEW_REFERENCED_CONTENT;
    $$->a.new_referenced_content = snaccAlloc(sizeof(NewReferencedContent));
    $$->a.new_referenced_content->generic_content_reference = $3;
    $$->a.new_referenced_content->new_content_size = $4;
    $$->a.new_referenced_content->new_content_cache_priority = $5;
}
;
NewContentSize: eTOK_TAG_NewContentSize GInt
{
    $$=snaccAlloc(sizeof(*$$));
    $$->choiceId = NEWCONTENTSIZE_CONTENT_SIZE;
    $$->a.content_size = $2;
}
| /* empty */ 
{
    $$=snaccAlloc(sizeof(*$$));
    $$->choiceId = NEWCONTENTSIZE_NULL;
}
;
NewContentCachePriority: eTOK_TAG_NewContentCachePriority GInt {$$=$2;}
| /* empty */ {$$=0;}
;
NewColour: eTOK_TAG_NewColourIndex GInt
{
    $$=snaccAlloc(sizeof(*$$));
    $$->choiceId = NEWCOLOUR_NEW_COLOUR_INDEX;
    $$->a.new_colour_index = $2;
}
| eTOK_TAG_NewAbsoluteColour GOString
{
    $$=snaccAlloc(sizeof(*$$));
    $$->choiceId = NEWCOLOUR_NEW_ABSOLUTE_COLOUR;
    $$->a.new_absolute_colour = $2;
}
;
NewColourOrNone: NewColour {$$=$1;}
| {$$=0;}
;
NewFont: eTOK_TAG_GenericOctetString GOString
{
    $$=snaccAlloc(sizeof(*$$));
    $$->choiceId = NEWFONT_NEW_FONT_NAME;
    $$->a.new_font_name = $2;
}
| eTOK_TAG_GenericObjectReference GObjRef
{
    $$=snaccAlloc(sizeof(*$$));
    $$->choiceId = NEWFONT_NEW_FONT_REFERENCE;
    $$->a.new_font_reference = $2;
}
;
Rational: GInt GInt
{
    $$=snaccAlloc(sizeof(*$$));
    $$->numerator = $1;
    $$->denominator = $2;
}
| GInt
{
    $$=snaccAlloc(sizeof(*$$));
    $$->numerator = $1;    
}
;
TimerValue: GInt {$$=$1;}
| /* empty */ {$$=0;}
;
AbsoluteTime: eTOK_TAG_AbsoluteTime GBool {$$=$2;}
| /* empty */ {$$=0;}
;
NewVariableValue: eTOK_TAG_GenericBoolean GBool
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = NEWVARIABLEVALUE_NEW_GENERIC_BOOLEAN;
    $$->a.new_generic_boolean = $2;
}
| eTOK_TAG_GenericInteger GInt
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = NEWVARIABLEVALUE_NEW_GENERIC_INTEGER;
    $$->a.new_generic_integer = $2;
}
| eTOK_TAG_GenericOctetString GOString
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = NEWVARIABLEVALUE_NEW_GENERIC_OCTET_STRING;
    $$->a.new_generic_octet_string = $2;
}
| eTOK_TAG_GenericObjectReference GObjRef
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = NEWVARIABLEVALUE_NEW_GENERIC_OBJECT_REFERENCE;
    $$->a.new_generic_object_reference = $2;
}
| eTOK_TAG_GenericContentReference GContRef
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = NEWVARIABLEVALUE_NEW_GENERIC_CONTENT_REFERENCE;
    $$->a.new_generic_content_reference = $2;
}
;
ComparisonOperator: GInt
{
    $$ = $1;
}
| eTOK_EXT_Equals
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = GENERICINTEGER_INTEGER;
    $$->a.integer = 1;
}
| eTOK_EXT_NotEquals
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = GENERICINTEGER_INTEGER;
    $$->a.integer = 2;
}
| eTOK_EXT_StrictlyLess
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = GENERICINTEGER_INTEGER;
    $$->a.integer = 3;
}
| eTOK_EXT_LessOrEqual
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = GENERICINTEGER_INTEGER;
    $$->a.integer = 4;
}
| eTOK_EXT_StrictlyGreater
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = GENERICINTEGER_INTEGER;
    $$->a.integer = 5;
}
| eTOK_EXT_GreaterOrEqual
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = GENERICINTEGER_INTEGER;
    $$->a.integer = 6;
}
;
ComparisonValue: eTOK_TAG_GenericBoolean GBool
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = COMPARISONVALUE_NEW_GENERIC_BOOLEAN;
    $$->a.new_generic_boolean = $2;
}
| eTOK_TAG_GenericInteger GInt
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = COMPARISONVALUE_NEW_GENERIC_INTEGER;
    $$->a.new_generic_integer = $2;
}
| eTOK_TAG_GenericOctetString GOString
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = COMPARISONVALUE_NEW_GENERIC_OCTETSTRING;
    $$->a.new_generic_octetstring = $2;
}
| eTOK_TAG_GenericObjectReference GObjRef
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = COMPARISONVALUE_NEW_GENERIC_OBJECT_REFERENCE;
    $$->a.new_generic_object_reference = $2;
}
| eTOK_TAG_GenericContentReference GContRef
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = COMPARISONVALUE_NEW_GENERIC_CONTENT_REFERENCE;
    $$->a.new_generic_content_reference = $2;
}
;

ConnectionTag: eTOK_TAG_ConnectionTag GInt {$$=$2;}
| /* empty */ {$$=0;}
;
TransitionEffect: GInt {$$=$1;}
| /* empty */ {$$=0;}
;

ObjectReference : eTOK_LPAREN eTOK_STRING Integer eTOK_RPAREN {$$=0;}
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = OBJECTREFERENCE_EXTERNAL_REFERENCE;
    $$->a.external_reference = snaccAlloc(sizeof(ExternalReference));
    $$->a.external_reference->group_identifier =  decode_string($2);
    $$->a.external_reference->object_number = $3;
}
| Integer
{
    $$ = snaccAlloc(sizeof(* $$));
    $$->choiceId = OBJECTREFERENCE_INTERNAL_REFERENCE;
    $$->a.internal_reference = $1;
}
;

GObjRef: ObjectReference
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = GENERICOBJECTREFERENCE_DIRECT_REFERENCE;
    $$->a.direct_reference = $1;
}
| eTOK_TAG_IndirectReference ObjectReference
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = GENERICOBJECTREFERENCE_INDIRECT_REFERENCE;
    $$->a.indirect_reference = $2;
}
;
GContRef: eTOK_STRING
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = GENERICCONTENTREFERENCE_CONTENT_REFERENCE;
    $$->a.content_reference = snaccAlloc(sizeof(ContentReference));
    *($$->a.content_reference) = decode_string($1);
}
| eTOK_TAG_IndirectReference ObjectReference
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = GENERICCONTENTREFERENCE_INDIRECT_REFERENCE;
    $$->a.indirect_reference = $2;
}
; 
GInt: Integer 
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = GENERICINTEGER_INTEGER;
    $$->a.integer = $1;
}
| eTOK_TAG_IndirectReference ObjectReference
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = GENERICINTEGER_INDIRECT_REFERENCE;
    $$->a.indirect_reference = $2;
}
;
GBool: eTOK_BOOLEAN 
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = GENERICBOOLEAN_BOOLEAN;
    $$->a.boolean = $1;
}
| eTOK_TAG_IndirectReference ObjectReference
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = GENERICBOOLEAN_INDIRECT_REFERENCE;
    $$->a.indirect_reference = $2;
}
;
GOString: eTOK_STRING 
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = GENERICOCTETSTRING_OCTETSTRING;
    $$->a.octetstring = snaccAlloc(sizeof(AsnOcts));
    *($$->a.octetstring) = decode_string($1);
}
| eTOK_TAG_IndirectReference ObjectReference
{
    $$ = snaccAlloc(sizeof(*$$));
    $$->choiceId = GENERICOCTETSTRING_INDIRECT_REFERENCE;
    $$->a.indirect_reference = $2;
}
;

Integer:  eTOK_INTEGER                      { $$ = $1; }
       | Integer eTOK_EXT_Plus Integer      { $$ = $1 + $3; }
       | Integer eTOK_EXT_Minus Integer     { $$ = $1 - $3; }
       | Integer eTOK_EXT_Multiply Integer  { $$ = $1 * $3; }
       | Integer eTOK_EXT_Divide Integer    { $$ = $1 / $3; }
       | eTOK_LPAREN Integer eTOK_RPAREN    { $$ = $2; }
;
%%

static void
yyerror(char *mess)
{
//    fprintf(stderr, "-- PARSE ERROR --: %s\n", mess);
    diag_msg(eSEVERITY_ERROR, mess, yylineno);
}
static void
warn_default(const char *field, int lineno)
{
    char msg_buf[256];
    sprintf(msg_buf, "Default value for %s need not be encoded", field);
    diag_msg(eSEVERITY_WARNING, msg_buf, lineno);
}

static void
warn_bad_field(const char *class, const char *field, int lineno)
{
    char msg_buf[256];
    sprintf(msg_buf, "Field %s not allowed in %s class", field, class);
    diag_msg(eSEVERITY_WARNING, msg_buf, lineno);
}

static AsnOcts
decode_string(AsnOcts in)
{
    AsnOcts out;
    /* resulting octetstring will always be smaller, so allocated in length */
    out.octs = snaccAlloc(in.octetLen);
    out.octetLen = 0;
            
    switch(in.octs[0])
    {
    case '\'':
        {
            char *ptr;
            char *optr;
            ptr = in.octs + 1;
            optr = out.octs;
            while(*ptr != '\''){
                if( *ptr == '=' ){
                    int nibble1, nibble2;
                    nibble1 = *(++ptr);
                    if( nibble1 <= '9' && nibble1 >= '0' )
                        nibble1 -= '0';
                    else if( nibble1 <= 'F' && nibble1 >= 'A' )
                        nibble1 = nibble1 - 'A' + 10;
                    else if( nibble1 <= 'f' && nibble1 >= 'a' )
                        nibble1 = nibble1 - 'a' + 10;
                    else if( nibble1 == '\n' )
                    {
                        ++ptr;
                        continue;
                    }
                    else
                    {
                        fprintf(stderr,
                                "Invalid QPRINTABLE string, skipping.\n");
                        out.octetLen = 0;
                        return out;
                    }
                    nibble2 = *(++ptr);
                    if( nibble2 <= '9' && nibble2 >= '0' )
                        nibble2 -= '0';
                    else if( nibble2 <= 'F' && nibble2 >= 'A' )
                        nibble2 = nibble2 - 'A' + 10;
                    else if( nibble2 <= 'f' && nibble2 >= 'a' )
                        nibble2 = nibble2 - 'a' + 10;
                    else
                    {
                        fprintf(stderr,
                                "Invalid QPRINTABLE string, skipping.\n");
                        out.octetLen = 0;
                        return out;
                    }
                    *optr++ = (unsigned char)(nibble1 << 4 | nibble2);
                    ptr++;
                }
                else
                    *optr++ = *ptr++;
                out.octetLen++;
            }
        }
        break;
    case '"':
        /* All we need do for strings is convert \\ and \" to \ and " */
        {
            char *ptr;
            char *optr;
            ptr = in.octs + 1;
            optr = out.octs;
            while(*ptr != '"'){
                if( *ptr == '\\' )
                {
                    if( *(++ptr) == '\\' )
                        *optr++ = '\\';
                    else 
                        *optr++ = '"';
                    /* NOTE: lexer will stop other cases getting through. */
                    ptr++;
                }
                else
                    *optr++ = *ptr++;
                out.octetLen++;
            }
        }      
        break;
    case '`':
        printf("xxNOT IMPLEMENTEDxx String is BASE64\n"); break;
        // BIG FAT TODO: BASE64 not in wide use, but needed anyway.
    }
    return out;
}

/* Object number hash table. Never meant to be freed!! */
/* --------------------------------------------------- */
#define OBJ_HASH_SIZE 17
#define HASH(x)       ((x) % OBJ_HASH_SIZE)
struct _hashItem {
    AsnInt num;
    int line;
    int reported;
    struct _hashItem *nxt;
};
static struct _hashItem* _hasTbl[OBJ_HASH_SIZE];


static void
add_object_number(AsnInt num, int lineno)
{
    AsnInt hash = HASH(num);
    struct _hashItem* ptr;

    for(ptr = _hasTbl[hash]; ptr; ptr = ptr->nxt)
    {
        if(!ptr)
            break;
        if(ptr->num == num)
        {
            diag_msg(eSEVERITY_WARNING, "Duplicate ObjectNumber", lineno);
            return;
        }
    }
    ptr = malloc(sizeof(*ptr));
    ptr->num = num;
    ptr->nxt = _hasTbl[hash];
    _hasTbl[hash] = ptr;
}


/*
 * Local variables:
 * compile-command: "make -C ../"
 *  tab-width: 4
 * End:
 */
