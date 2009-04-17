/*
 * asn1tag.h
 */

/*
 * Copyright (C) 2007, Simon Kilvington
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef __ASN1TAG_H__
#define __ASN1TAG_H__

#include <stdbool.h>

/* tag classes */
#define ASN1CLASS_UNIVERSAL	0x00
#define ASN1CLASS_APPLICATION	0x40
#define ASN1CLASS_CONTEXT	0x80
#define ASN1CLASS_PRIVATE	0xc0

char *asn1class_name(unsigned int);
bool is_synthetic(unsigned int);
bool needs_tagging(unsigned int, unsigned int);
bool keep_tag(unsigned int);
unsigned int asn1tagclass(const char *);

/*
 * a synthetic object created as a result of the grammar definition
 * eg TextBody etc
 * don't output this object, just output its children
 */
#define ASN1TAG_SYNTHETIC	10000
/* the tag for CHOICE types is determined by which choice we choose */
#define ASN1TAG_CHOICE		10001
/* for signalling internal parser errors */
#define ASN1TAG_BAD		10002

/* abstract types */
#define ASN1TAGCLASS_Root		ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_Group		ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_Presentable	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_Ingredient		ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_Program		ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_Variable		ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_Visible		ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_Interactible	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_Button		ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_TokenManager	ASN1TAG_SYNTHETIC

/* tokens synthesised by the grammar */
#define ASN1TAGCLASS_ObjectIdentifier	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_TokenGroupBody	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_LineArtBody	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_TextBody		ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_PushButtonBody	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_BoxSize		ASN1TAG_SYNTHETIC

/*
 * this is not the same as an ASN1 OCTET STRING type
 * rather it a choice about how you define the data in your source file
 * ie as a STRING, QPRINTABLE or BASE64
 */
#define ASN1TAGCLASS_OctetString	ASN1TAG_CHOICE

/* ASN1 CONTEXT tag values */
#define ASN1TAG_ApplicationClass	0
#define ASN1TAG_SceneClass	1
#define ASN1TAG_StandardIdentifier	2
#define ASN1TAG_StandardVersion	3
#define ASN1TAG_ObjectInformation	4
#define ASN1TAG_OnStartUp	5
#define ASN1TAG_OnCloseDown	6
#define ASN1TAG_OriginalGroupCachePriority	7
#define ASN1TAG_Items	8
#define ASN1TAG_ResidentProgramClass	9
#define ASN1TAG_RemoteProgramClass	10
#define ASN1TAG_InterchangedProgramClass	11
#define ASN1TAG_PaletteClass	12
#define ASN1TAG_FontClass	13
#define ASN1TAG_CursorShapeClass	14
#define ASN1TAG_BooleanVariableClass	15
#define ASN1TAG_IntegerVariableClass	16
#define ASN1TAG_OctetStringVariableClass	17
#define ASN1TAG_ObjectRefVariableClass	18
#define ASN1TAG_ContentRefVariableClass	19
#define ASN1TAG_LinkClass	20
#define ASN1TAG_StreamClass	21
#define ASN1TAG_BitmapClass	22
#define ASN1TAG_LineArtClass	23
#define ASN1TAG_DynamicLineArtClass	24
#define ASN1TAG_RectangleClass	25
#define ASN1TAG_HotspotClass	26
#define ASN1TAG_SwitchButtonClass	27
#define ASN1TAG_PushButtonClass	28
#define ASN1TAG_TextClass	29
#define ASN1TAG_EntryFieldClass	30
#define ASN1TAG_HyperTextClass	31
#define ASN1TAG_SliderClass	32
#define ASN1TAG_TokenGroupClass	33
#define ASN1TAG_ListGroupClass	34
#define ASN1TAG_OnSpawnCloseDown	35
#define ASN1TAG_OnRestart	36
#define ASN1TAG_DefaultAttributes	37
#define ASN1TAG_CharacterSet	38
#define ASN1TAG_BackgroundColour	39
#define ASN1TAG_TextContentHook	40
#define ASN1TAG_TextColour	41
#define ASN1TAG_Font	42
#define ASN1TAG_FontAttributes	43
#define ASN1TAG_InterchangedProgramContentHook	44
#define ASN1TAG_StreamContentHook	45
#define ASN1TAG_BitmapContentHook	46
#define ASN1TAG_LineArtContentHook	47
#define ASN1TAG_ButtonRefColour	48
#define ASN1TAG_HighlightRefColour	49
#define ASN1TAG_SliderRefColour	50
#define ASN1TAG_InputEventRegister	51
#define ASN1TAG_SceneCoordinateSystem	52
#define ASN1TAG_AspectRatio	53
#define ASN1TAG_MovingCursor	54
#define ASN1TAG_NextScenes	55
#define ASN1TAG_InitiallyActive	56
#define ASN1TAG_ContentHook	57
#define ASN1TAG_OriginalContent	58
#define ASN1TAG_Shared	59
#define ASN1TAG_ContentSize	60
#define ASN1TAG_ContentCachePriority	61
#define ASN1TAG_LinkCondition	62
#define ASN1TAG_LinkEffect	63
#define ASN1TAG_Name	64
#define ASN1TAG_InitiallyAvailable	65
#define ASN1TAG_ProgramConnectionTag	66
#define ASN1TAG_OriginalValue	67
#define ASN1TAG_ObjectReferenceValue	68
#define ASN1TAG_ContentReference69	69
#define ASN1TAG_MovementTable	70
#define ASN1TAG_TokenGroupItems	71
#define ASN1TAG_NoTokenActionSlots	72
#define ASN1TAG_Positions	73
#define ASN1TAG_WrapAround	74
#define ASN1TAG_MultipleSelection	75
#define ASN1TAG_OriginalBoxSize	76
#define ASN1TAG_OriginalPosition	77
#define ASN1TAG_OriginalPaletteRef	78
#define ASN1TAG_Tiling	79
#define ASN1TAG_OriginalTransparency	80
#define ASN1TAG_BorderedBoundingBox	81
#define ASN1TAG_OriginalLineWidth	82
#define ASN1TAG_OriginalLineStyle	83
#define ASN1TAG_OriginalRefLineColour	84
#define ASN1TAG_OriginalRefFillColour	85
#define ASN1TAG_OriginalFont	86
#define ASN1TAG_HorizontalJustification	87
#define ASN1TAG_VerticalJustification	88
#define ASN1TAG_LineOrientation	89
#define ASN1TAG_StartCorner	90
#define ASN1TAG_TextWrapping	91
#define ASN1TAG_Multiplex	92
#define ASN1TAG_Storage	93
#define ASN1TAG_Looping	94
#define ASN1TAG_AudioClass	95
#define ASN1TAG_VideoClass	96
#define ASN1TAG_RTGraphicsClass	97
#define ASN1TAG_ComponentTag	98
#define ASN1TAG_OriginalVolume	99
#define ASN1TAG_Termination	100
#define ASN1TAG_EngineResp	101
#define ASN1TAG_Orientation	102
#define ASN1TAG_MaxValue	103
#define ASN1TAG_MinValue	104
#define ASN1TAG_InitialValue	105
#define ASN1TAG_InitialPortion	106
#define ASN1TAG_StepSize	107
#define ASN1TAG_SliderStyle	108
#define ASN1TAG_InputType	109
#define ASN1TAG_CharList	110
#define ASN1TAG_ObscuredInput	111
#define ASN1TAG_MaxLength	112
#define ASN1TAG_OriginalLabel	113
#define ASN1TAG_ButtonStyle	114
#define ASN1TAG_Activate	115
#define ASN1TAG_Add	116
#define ASN1TAG_AddItem	117
#define ASN1TAG_Append	118
#define ASN1TAG_BringToFront	119
#define ASN1TAG_Call	120
#define ASN1TAG_CallActionSlot	121
#define ASN1TAG_Clear	122
#define ASN1TAG_Clone	123
#define ASN1TAG_CloseConnection	124
#define ASN1TAG_Deactivate	125
#define ASN1TAG_DelItem	126
#define ASN1TAG_Deselect	127
#define ASN1TAG_DeselectItem	128
#define ASN1TAG_Divide	129
#define ASN1TAG_DrawArc	130
#define ASN1TAG_DrawLine	131
#define ASN1TAG_DrawOval	132
#define ASN1TAG_DrawPolygon	133
#define ASN1TAG_DrawPolyline	134
#define ASN1TAG_DrawRectangle	135
#define ASN1TAG_DrawSector	136
#define ASN1TAG_Fork	137
#define ASN1TAG_GetAvailabilityStatus	138
#define ASN1TAG_GetBoxSize	139
#define ASN1TAG_GetCellItem	140
#define ASN1TAG_GetCursorPosition	141
#define ASN1TAG_GetEngineSupport	142
#define ASN1TAG_GetEntryPoint	143
#define ASN1TAG_GetFillColour	144
#define ASN1TAG_GetFirstItem	145
#define ASN1TAG_GetHighlightStatus	146
#define ASN1TAG_GetInteractionStatus	147
#define ASN1TAG_GetItemStatus	148
#define ASN1TAG_GetLabel	149
#define ASN1TAG_GetLastAnchorFired	150
#define ASN1TAG_GetLineColour	151
#define ASN1TAG_GetLineStyle	152
#define ASN1TAG_GetLineWidth	153
#define ASN1TAG_GetListItem	154
#define ASN1TAG_GetListSize	155
#define ASN1TAG_GetOverwriteMode	156
#define ASN1TAG_GetPortion	157
#define ASN1TAG_GetPosition	158
#define ASN1TAG_GetRunningStatus	159
#define ASN1TAG_GetSelectionStatus	160
#define ASN1TAG_GetSliderValue	161
#define ASN1TAG_GetTextContent	162
#define ASN1TAG_GetTextData	163
#define ASN1TAG_GetTokenPosition	164
#define ASN1TAG_GetVolume	165
#define ASN1TAG_Launch	166
#define ASN1TAG_LockScreen	167
#define ASN1TAG_Modulo	168
#define ASN1TAG_Move	169
#define ASN1TAG_MoveTo	170
#define ASN1TAG_Multiply	171
#define ASN1TAG_OpenConnection	172
#define ASN1TAG_Preload	173
#define ASN1TAG_PutBefore	174
#define ASN1TAG_PutBehind	175
#define ASN1TAG_Quit	176
#define ASN1TAG_ReadPersistent	177
#define ASN1TAG_Run	178
#define ASN1TAG_ScaleBitmap	179
#define ASN1TAG_ScaleVideo	180
#define ASN1TAG_ScrollItems	181
#define ASN1TAG_Select	182
#define ASN1TAG_SelectItem	183
#define ASN1TAG_SendEvent	184
#define ASN1TAG_SendToBack	185
#define ASN1TAG_SetBoxSize	186
#define ASN1TAG_SetCachePriority	187
#define ASN1TAG_SetCounterEndPosition	188
#define ASN1TAG_SetCounterPosition	189
#define ASN1TAG_SetCounterTrigger	190
#define ASN1TAG_SetCursorPosition	191
#define ASN1TAG_SetCursorShape	192
#define ASN1TAG_SetData	193
#define ASN1TAG_SetEntryPoint	194
#define ASN1TAG_SetFillColour	195
#define ASN1TAG_SetFirstItem	196
#define ASN1TAG_SetFontRef	197
#define ASN1TAG_SetHighlightStatus	198
#define ASN1TAG_SetInteractionStatus	199
#define ASN1TAG_SetLabel	200
#define ASN1TAG_SetLineColour	201
#define ASN1TAG_SetLineStyle	202
#define ASN1TAG_SetLineWidth	203
#define ASN1TAG_SetOverwriteMode	204
#define ASN1TAG_SetPaletteRef	205
#define ASN1TAG_SetPortion	206
#define ASN1TAG_SetPosition	207
#define ASN1TAG_SetSliderValue	208
#define ASN1TAG_SetSpeed	209
#define ASN1TAG_SetTimer	210
#define ASN1TAG_SetTransparency	211
#define ASN1TAG_SetVariable	212
#define ASN1TAG_SetVolume	213
#define ASN1TAG_Spawn	214
#define ASN1TAG_Step	215
#define ASN1TAG_Stop	216
#define ASN1TAG_StorePersistent	217
#define ASN1TAG_Subtract	218
#define ASN1TAG_TestVariable	219
#define ASN1TAG_Toggle	220
#define ASN1TAG_ToggleItem	221
#define ASN1TAG_TransitionTo	222
#define ASN1TAG_Unload	223
#define ASN1TAG_UnlockScreen	224
#define ASN1TAG_NewGenericBoolean	225
#define ASN1TAG_NewGenericInteger	226
#define ASN1TAG_NewGenericOctetString	227
#define ASN1TAG_NewGenericObjectReference	228
#define ASN1TAG_NewGenericContentReference	229
#define ASN1TAG_NewColourIndex	230
#define ASN1TAG_NewAbsoluteColour	231
#define ASN1TAG_NewFontName	232
#define ASN1TAG_NewFontReference	233
#define ASN1TAG_NewContentSize	234
#define ASN1TAG_NewContentCachePriority	235
#define ASN1TAG_IndirectReference	236
#define ASN1TAG_SetBackgroundColour	237
#define ASN1TAG_SetCellPosition	238
#define ASN1TAG_SetInputReg	239
#define ASN1TAG_SetTextColour	240
#define ASN1TAG_SetFontAttributes	241
#define ASN1TAG_SetVideoDecodeOffset	242
#define ASN1TAG_GetVideoDecodeOffset	243
#define ASN1TAG_GetFocusPosition	244
#define ASN1TAG_SetFocusPosition	245
#define ASN1TAG_SetBitmapDecodeOffset	246
#define ASN1TAG_GetBitmapDecodeOffset	247
#define ASN1TAG_SetSliderParameters	248

/* tag and class in a single value */
#define ASN1TAGCLASS_ApplicationClass	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_ApplicationClass)
#define ASN1TAGCLASS_SceneClass	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_SceneClass)
#define ASN1TAGCLASS_StandardIdentifier	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_StandardIdentifier)
#define ASN1TAGCLASS_StandardVersion	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_StandardVersion)
#define ASN1TAGCLASS_ObjectInformation	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_ObjectInformation)
#define ASN1TAGCLASS_OnStartUp	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_OnStartUp)
#define ASN1TAGCLASS_OnCloseDown	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_OnCloseDown)
#define ASN1TAGCLASS_OriginalGroupCachePriority	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_OriginalGroupCachePriority)
#define ASN1TAGCLASS_Items	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_Items)
#define ASN1TAGCLASS_ResidentProgramClass	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_ResidentProgramClass)
#define ASN1TAGCLASS_RemoteProgramClass	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_RemoteProgramClass)
#define ASN1TAGCLASS_InterchangedProgramClass	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_InterchangedProgramClass)
#define ASN1TAGCLASS_PaletteClass	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_PaletteClass)
#define ASN1TAGCLASS_FontClass	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_FontClass)
#define ASN1TAGCLASS_CursorShapeClass	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_CursorShapeClass)
#define ASN1TAGCLASS_BooleanVariableClass	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_BooleanVariableClass)
#define ASN1TAGCLASS_IntegerVariableClass	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_IntegerVariableClass)
#define ASN1TAGCLASS_OctetStringVariableClass	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_OctetStringVariableClass)
#define ASN1TAGCLASS_ObjectRefVariableClass	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_ObjectRefVariableClass)
#define ASN1TAGCLASS_ContentRefVariableClass	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_ContentRefVariableClass)
#define ASN1TAGCLASS_LinkClass	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_LinkClass)
#define ASN1TAGCLASS_StreamClass	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_StreamClass)
#define ASN1TAGCLASS_BitmapClass	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_BitmapClass)
#define ASN1TAGCLASS_LineArtClass	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_LineArtClass)
#define ASN1TAGCLASS_DynamicLineArtClass	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_DynamicLineArtClass)
#define ASN1TAGCLASS_RectangleClass	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_RectangleClass)
#define ASN1TAGCLASS_HotspotClass	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_HotspotClass)
#define ASN1TAGCLASS_SwitchButtonClass	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_SwitchButtonClass)
#define ASN1TAGCLASS_PushButtonClass	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_PushButtonClass)
#define ASN1TAGCLASS_TextClass	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_TextClass)
#define ASN1TAGCLASS_EntryFieldClass	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_EntryFieldClass)
#define ASN1TAGCLASS_HyperTextClass	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_HyperTextClass)
#define ASN1TAGCLASS_SliderClass	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_SliderClass)
#define ASN1TAGCLASS_TokenGroupClass	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_TokenGroupClass)
#define ASN1TAGCLASS_ListGroupClass	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_ListGroupClass)
#define ASN1TAGCLASS_OnSpawnCloseDown	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_OnSpawnCloseDown)
#define ASN1TAGCLASS_OnRestart	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_OnRestart)
#define ASN1TAGCLASS_DefaultAttributes	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_DefaultAttributes)
#define ASN1TAGCLASS_CharacterSet	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_CharacterSet)
#define ASN1TAGCLASS_BackgroundColour	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_BackgroundColour)
#define ASN1TAGCLASS_TextContentHook	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_TextContentHook)
#define ASN1TAGCLASS_TextColour	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_TextColour)
#define ASN1TAGCLASS_Font	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_Font)
#define ASN1TAGCLASS_FontAttributes	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_FontAttributes)
#define ASN1TAGCLASS_InterchangedProgramContentHook	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_InterchangedProgramContentHook)
#define ASN1TAGCLASS_StreamContentHook	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_StreamContentHook)
#define ASN1TAGCLASS_BitmapContentHook	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_BitmapContentHook)
#define ASN1TAGCLASS_LineArtContentHook	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_LineArtContentHook)
#define ASN1TAGCLASS_ButtonRefColour	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_ButtonRefColour)
#define ASN1TAGCLASS_HighlightRefColour	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_HighlightRefColour)
#define ASN1TAGCLASS_SliderRefColour	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_SliderRefColour)
#define ASN1TAGCLASS_InputEventRegister	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_InputEventRegister)
#define ASN1TAGCLASS_SceneCoordinateSystem	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_SceneCoordinateSystem)
#define ASN1TAGCLASS_AspectRatio	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_AspectRatio)
#define ASN1TAGCLASS_MovingCursor	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_MovingCursor)
#define ASN1TAGCLASS_NextScenes	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_NextScenes)
#define ASN1TAGCLASS_InitiallyActive	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_InitiallyActive)
#define ASN1TAGCLASS_ContentHook	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_ContentHook)
#define ASN1TAGCLASS_OriginalContent	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_OriginalContent)
#define ASN1TAGCLASS_Shared	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_Shared)
#define ASN1TAGCLASS_ContentSize	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_ContentSize)
#define ASN1TAGCLASS_ContentCachePriority	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_ContentCachePriority)
#define ASN1TAGCLASS_LinkCondition	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_LinkCondition)
#define ASN1TAGCLASS_LinkEffect	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_LinkEffect)
#define ASN1TAGCLASS_Name	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_Name)
#define ASN1TAGCLASS_InitiallyAvailable	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_InitiallyAvailable)
#define ASN1TAGCLASS_ProgramConnectionTag	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_ProgramConnectionTag)
#define ASN1TAGCLASS_OriginalValue	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_OriginalValue)
#define ASN1TAGCLASS_ObjectReferenceValue	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_ObjectReferenceValue)
#define ASN1TAGCLASS_ContentReference69	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_ContentReference69)
#define ASN1TAGCLASS_MovementTable	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_MovementTable)
#define ASN1TAGCLASS_TokenGroupItems	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_TokenGroupItems)
#define ASN1TAGCLASS_NoTokenActionSlots	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_NoTokenActionSlots)
#define ASN1TAGCLASS_Positions	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_Positions)
#define ASN1TAGCLASS_WrapAround	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_WrapAround)
#define ASN1TAGCLASS_MultipleSelection	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_MultipleSelection)
#define ASN1TAGCLASS_OriginalBoxSize	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_OriginalBoxSize)
#define ASN1TAGCLASS_OriginalPosition	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_OriginalPosition)
#define ASN1TAGCLASS_OriginalPaletteRef	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_OriginalPaletteRef)
#define ASN1TAGCLASS_Tiling	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_Tiling)
#define ASN1TAGCLASS_OriginalTransparency	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_OriginalTransparency)
#define ASN1TAGCLASS_BorderedBoundingBox	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_BorderedBoundingBox)
#define ASN1TAGCLASS_OriginalLineWidth	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_OriginalLineWidth)
#define ASN1TAGCLASS_OriginalLineStyle	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_OriginalLineStyle)
#define ASN1TAGCLASS_OriginalRefLineColour	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_OriginalRefLineColour)
#define ASN1TAGCLASS_OriginalRefFillColour	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_OriginalRefFillColour)
#define ASN1TAGCLASS_OriginalFont	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_OriginalFont)
#define ASN1TAGCLASS_HorizontalJustification	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_HorizontalJustification)
#define ASN1TAGCLASS_VerticalJustification	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_VerticalJustification)
#define ASN1TAGCLASS_LineOrientation	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_LineOrientation)
#define ASN1TAGCLASS_StartCorner	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_StartCorner)
#define ASN1TAGCLASS_TextWrapping	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_TextWrapping)
#define ASN1TAGCLASS_Multiplex	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_Multiplex)
#define ASN1TAGCLASS_Storage	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_Storage)
#define ASN1TAGCLASS_Looping	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_Looping)
#define ASN1TAGCLASS_AudioClass	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_AudioClass)
#define ASN1TAGCLASS_VideoClass	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_VideoClass)
#define ASN1TAGCLASS_RTGraphicsClass	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_RTGraphicsClass)
#define ASN1TAGCLASS_ComponentTag	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_ComponentTag)
#define ASN1TAGCLASS_OriginalVolume	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_OriginalVolume)
#define ASN1TAGCLASS_Termination	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_Termination)
#define ASN1TAGCLASS_EngineResp	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_EngineResp)
#define ASN1TAGCLASS_Orientation	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_Orientation)
#define ASN1TAGCLASS_MaxValue	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_MaxValue)
#define ASN1TAGCLASS_MinValue	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_MinValue)
#define ASN1TAGCLASS_InitialValue	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_InitialValue)
#define ASN1TAGCLASS_InitialPortion	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_InitialPortion)
#define ASN1TAGCLASS_StepSize	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_StepSize)
#define ASN1TAGCLASS_SliderStyle	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_SliderStyle)
#define ASN1TAGCLASS_InputType	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_InputType)
#define ASN1TAGCLASS_CharList	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_CharList)
#define ASN1TAGCLASS_ObscuredInput	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_ObscuredInput)
#define ASN1TAGCLASS_MaxLength	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_MaxLength)
#define ASN1TAGCLASS_OriginalLabel	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_OriginalLabel)
#define ASN1TAGCLASS_ButtonStyle	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_ButtonStyle)
#define ASN1TAGCLASS_Activate	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_Activate)
#define ASN1TAGCLASS_Add	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_Add)
#define ASN1TAGCLASS_AddItem	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_AddItem)
#define ASN1TAGCLASS_Append	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_Append)
#define ASN1TAGCLASS_BringToFront	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_BringToFront)
#define ASN1TAGCLASS_Call	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_Call)
#define ASN1TAGCLASS_CallActionSlot	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_CallActionSlot)
#define ASN1TAGCLASS_Clear	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_Clear)
#define ASN1TAGCLASS_Clone	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_Clone)
#define ASN1TAGCLASS_CloseConnection	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_CloseConnection)
#define ASN1TAGCLASS_Deactivate	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_Deactivate)
#define ASN1TAGCLASS_DelItem	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_DelItem)
#define ASN1TAGCLASS_Deselect	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_Deselect)
#define ASN1TAGCLASS_DeselectItem	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_DeselectItem)
#define ASN1TAGCLASS_Divide	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_Divide)
#define ASN1TAGCLASS_DrawArc	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_DrawArc)
#define ASN1TAGCLASS_DrawLine	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_DrawLine)
#define ASN1TAGCLASS_DrawOval	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_DrawOval)
#define ASN1TAGCLASS_DrawPolygon	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_DrawPolygon)
#define ASN1TAGCLASS_DrawPolyline	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_DrawPolyline)
#define ASN1TAGCLASS_DrawRectangle	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_DrawRectangle)
#define ASN1TAGCLASS_DrawSector	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_DrawSector)
#define ASN1TAGCLASS_Fork	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_Fork)
#define ASN1TAGCLASS_GetAvailabilityStatus	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_GetAvailabilityStatus)
#define ASN1TAGCLASS_GetBoxSize	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_GetBoxSize)
#define ASN1TAGCLASS_GetCellItem	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_GetCellItem)
#define ASN1TAGCLASS_GetCursorPosition	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_GetCursorPosition)
#define ASN1TAGCLASS_GetEngineSupport	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_GetEngineSupport)
#define ASN1TAGCLASS_GetEntryPoint	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_GetEntryPoint)
#define ASN1TAGCLASS_GetFillColour	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_GetFillColour)
#define ASN1TAGCLASS_GetFirstItem	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_GetFirstItem)
#define ASN1TAGCLASS_GetHighlightStatus	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_GetHighlightStatus)
#define ASN1TAGCLASS_GetInteractionStatus	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_GetInteractionStatus)
#define ASN1TAGCLASS_GetItemStatus	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_GetItemStatus)
#define ASN1TAGCLASS_GetLabel	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_GetLabel)
#define ASN1TAGCLASS_GetLastAnchorFired	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_GetLastAnchorFired)
#define ASN1TAGCLASS_GetLineColour	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_GetLineColour)
#define ASN1TAGCLASS_GetLineStyle	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_GetLineStyle)
#define ASN1TAGCLASS_GetLineWidth	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_GetLineWidth)
#define ASN1TAGCLASS_GetListItem	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_GetListItem)
#define ASN1TAGCLASS_GetListSize	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_GetListSize)
#define ASN1TAGCLASS_GetOverwriteMode	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_GetOverwriteMode)
#define ASN1TAGCLASS_GetPortion	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_GetPortion)
#define ASN1TAGCLASS_GetPosition	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_GetPosition)
#define ASN1TAGCLASS_GetRunningStatus	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_GetRunningStatus)
#define ASN1TAGCLASS_GetSelectionStatus	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_GetSelectionStatus)
#define ASN1TAGCLASS_GetSliderValue	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_GetSliderValue)
#define ASN1TAGCLASS_GetTextContent	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_GetTextContent)
#define ASN1TAGCLASS_GetTextData	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_GetTextData)
#define ASN1TAGCLASS_GetTokenPosition	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_GetTokenPosition)
#define ASN1TAGCLASS_GetVolume	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_GetVolume)
#define ASN1TAGCLASS_Launch	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_Launch)
#define ASN1TAGCLASS_LockScreen	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_LockScreen)
#define ASN1TAGCLASS_Modulo	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_Modulo)
#define ASN1TAGCLASS_Move	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_Move)
#define ASN1TAGCLASS_MoveTo	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_MoveTo)
#define ASN1TAGCLASS_Multiply	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_Multiply)
#define ASN1TAGCLASS_OpenConnection	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_OpenConnection)
#define ASN1TAGCLASS_Preload	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_Preload)
#define ASN1TAGCLASS_PutBefore	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_PutBefore)
#define ASN1TAGCLASS_PutBehind	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_PutBehind)
#define ASN1TAGCLASS_Quit	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_Quit)
#define ASN1TAGCLASS_ReadPersistent	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_ReadPersistent)
#define ASN1TAGCLASS_Run	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_Run)
#define ASN1TAGCLASS_ScaleBitmap	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_ScaleBitmap)
#define ASN1TAGCLASS_ScaleVideo	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_ScaleVideo)
#define ASN1TAGCLASS_ScrollItems	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_ScrollItems)
#define ASN1TAGCLASS_Select	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_Select)
#define ASN1TAGCLASS_SelectItem	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_SelectItem)
#define ASN1TAGCLASS_SendEvent	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_SendEvent)
#define ASN1TAGCLASS_SendToBack	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_SendToBack)
#define ASN1TAGCLASS_SetBoxSize	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_SetBoxSize)
#define ASN1TAGCLASS_SetCachePriority	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_SetCachePriority)
#define ASN1TAGCLASS_SetCounterEndPosition	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_SetCounterEndPosition)
#define ASN1TAGCLASS_SetCounterPosition	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_SetCounterPosition)
#define ASN1TAGCLASS_SetCounterTrigger	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_SetCounterTrigger)
#define ASN1TAGCLASS_SetCursorPosition	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_SetCursorPosition)
#define ASN1TAGCLASS_SetCursorShape	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_SetCursorShape)
#define ASN1TAGCLASS_SetData	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_SetData)
#define ASN1TAGCLASS_SetEntryPoint	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_SetEntryPoint)
#define ASN1TAGCLASS_SetFillColour	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_SetFillColour)
#define ASN1TAGCLASS_SetFirstItem	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_SetFirstItem)
#define ASN1TAGCLASS_SetFontRef	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_SetFontRef)
#define ASN1TAGCLASS_SetHighlightStatus	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_SetHighlightStatus)
#define ASN1TAGCLASS_SetInteractionStatus	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_SetInteractionStatus)
#define ASN1TAGCLASS_SetLabel	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_SetLabel)
#define ASN1TAGCLASS_SetLineColour	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_SetLineColour)
#define ASN1TAGCLASS_SetLineStyle	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_SetLineStyle)
#define ASN1TAGCLASS_SetLineWidth	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_SetLineWidth)
#define ASN1TAGCLASS_SetOverwriteMode	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_SetOverwriteMode)
#define ASN1TAGCLASS_SetPaletteRef	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_SetPaletteRef)
#define ASN1TAGCLASS_SetPortion	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_SetPortion)
#define ASN1TAGCLASS_SetPosition	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_SetPosition)
#define ASN1TAGCLASS_SetSliderValue	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_SetSliderValue)
#define ASN1TAGCLASS_SetSpeed	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_SetSpeed)
#define ASN1TAGCLASS_SetTimer	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_SetTimer)
#define ASN1TAGCLASS_SetTransparency	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_SetTransparency)
#define ASN1TAGCLASS_SetVariable	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_SetVariable)
#define ASN1TAGCLASS_SetVolume	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_SetVolume)
#define ASN1TAGCLASS_Spawn	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_Spawn)
#define ASN1TAGCLASS_Step	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_Step)
#define ASN1TAGCLASS_Stop	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_Stop)
#define ASN1TAGCLASS_StorePersistent	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_StorePersistent)
#define ASN1TAGCLASS_Subtract	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_Subtract)
#define ASN1TAGCLASS_TestVariable	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_TestVariable)
#define ASN1TAGCLASS_Toggle	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_Toggle)
#define ASN1TAGCLASS_ToggleItem	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_ToggleItem)
#define ASN1TAGCLASS_TransitionTo	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_TransitionTo)
#define ASN1TAGCLASS_Unload	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_Unload)
#define ASN1TAGCLASS_UnlockScreen	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_UnlockScreen)
#define ASN1TAGCLASS_NewGenericBoolean	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_NewGenericBoolean)
#define ASN1TAGCLASS_NewGenericInteger	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_NewGenericInteger)
#define ASN1TAGCLASS_NewGenericOctetString	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_NewGenericOctetString)
#define ASN1TAGCLASS_NewGenericObjectReference	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_NewGenericObjectReference)
#define ASN1TAGCLASS_NewGenericContentReference	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_NewGenericContentReference)
#define ASN1TAGCLASS_NewColourIndex	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_NewColourIndex)
#define ASN1TAGCLASS_NewAbsoluteColour	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_NewAbsoluteColour)
#define ASN1TAGCLASS_NewFontName	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_NewFontName)
#define ASN1TAGCLASS_NewFontReference	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_NewFontReference)
#define ASN1TAGCLASS_NewContentSize	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_NewContentSize)
#define ASN1TAGCLASS_NewContentCachePriority	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_NewContentCachePriority)
#define ASN1TAGCLASS_IndirectReference	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_IndirectReference)
#define ASN1TAGCLASS_SetBackgroundColour	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_SetBackgroundColour)
#define ASN1TAGCLASS_SetCellPosition	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_SetCellPosition)
#define ASN1TAGCLASS_SetInputReg	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_SetInputReg)
#define ASN1TAGCLASS_SetTextColour	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_SetTextColour)
#define ASN1TAGCLASS_SetFontAttributes	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_SetFontAttributes)
#define ASN1TAGCLASS_SetVideoDecodeOffset	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_SetVideoDecodeOffset)
#define ASN1TAGCLASS_GetVideoDecodeOffset	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_GetVideoDecodeOffset)
#define ASN1TAGCLASS_GetFocusPosition	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_GetFocusPosition)
#define ASN1TAGCLASS_SetFocusPosition	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_SetFocusPosition)
#define ASN1TAGCLASS_SetBitmapDecodeOffset	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_SetBitmapDecodeOffset)
#define ASN1TAGCLASS_GetBitmapDecodeOffset	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_GetBitmapDecodeOffset)
#define ASN1TAGCLASS_SetSliderParameters	((ASN1CLASS_CONTEXT << 24) | ASN1TAG_SetSliderParameters)

/* ASN1 UNIVERSAL tag values */
#define ASN1TAG_BOOLEAN		1
#define ASN1TAG_INTEGER		2
#define ASN1TAG_OCTETSTRING	4
#define ASN1TAG_NULL		5
#define ASN1TAG_ENUMERATED	10
#define ASN1TAG_SEQUENCE	16
#define ASN1TAG_SET		17
/* and with the class included */
#define ASN1TAGCLASS_BOOLEAN		((ASN1CLASS_UNIVERSAL << 24) | ASN1TAG_BOOLEAN)
#define ASN1TAGCLASS_INTEGER		((ASN1CLASS_UNIVERSAL << 24) | ASN1TAG_INTEGER)
#define ASN1TAGCLASS_OCTETSTRING	((ASN1CLASS_UNIVERSAL << 24) | ASN1TAG_OCTETSTRING)
#define ASN1TAGCLASS_NULL		((ASN1CLASS_UNIVERSAL << 24) | ASN1TAG_NULL)
#define ASN1TAGCLASS_ENUMERATED		((ASN1CLASS_UNIVERSAL << 24) | ASN1TAG_ENUMERATED)
#define ASN1TAGCLASS_SEQUENCE		((ASN1CLASS_UNIVERSAL << 24) | ASN1TAG_SEQUENCE)
#define ASN1TAGCLASS_SET		((ASN1CLASS_UNIVERSAL << 24) | ASN1TAG_SET)
/* alias */
#define ASN1TAGCLASS_Null		ASN1TAGCLASS_NULL

/* the ASN1 types in the grammar that are not CONTEXT class types */
#define ASN1TAGCLASS_JointIsoItuIdentifier	ASN1TAGCLASS_INTEGER
#define ASN1TAGCLASS_MHEGStandardIdentifier	ASN1TAGCLASS_INTEGER
#define ASN1TAGCLASS_DirectFont	ASN1TAGCLASS_OctetString
#define ASN1TAGCLASS_IndirectFont	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_XScene	ASN1TAGCLASS_INTEGER
#define ASN1TAGCLASS_YScene	ASN1TAGCLASS_INTEGER
#define ASN1TAGCLASS_Width	ASN1TAGCLASS_INTEGER
#define ASN1TAGCLASS_Height	ASN1TAGCLASS_INTEGER
#define ASN1TAGCLASS_SceneRef	ASN1TAGCLASS_OctetString
#define ASN1TAGCLASS_SceneWeight	ASN1TAGCLASS_INTEGER
#define ASN1TAGCLASS_IncludedContent	ASN1TAGCLASS_OctetString
#define ASN1TAGCLASS_EventSource	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_EventType	ASN1TAGCLASS_ENUMERATED
#define ASN1TAGCLASS_EventData	ASN1TAG_CHOICE
#define ASN1TAGCLASS_ObjectReference	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_ContentReferenceValue	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_TargetElement	ASN1TAGCLASS_INTEGER
#define ASN1TAGCLASS_AVisible	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_Position	ASN1TAGCLASS_SEQUENCE
#define ASN1TAGCLASS_XLength	ASN1TAGCLASS_INTEGER
#define ASN1TAGCLASS_YLength	ASN1TAGCLASS_INTEGER
#define ASN1TAGCLASS_AbsoluteTime	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_Address	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_Answer	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_AppendValue	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_ArcAngle	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_AvailabilityStatusVar	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_CallSucceeded	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_CellIndex	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_CloneRefVar	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_ConnectionTag	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_Denominator	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_EllipseHeight	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_EllipseWidth	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_EmulatedEventSource	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_EmulatedEventType	ASN1TAGCLASS_ENUMERATED
#define ASN1TAGCLASS_EntryPointVar	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_ForkSucceeded	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_Feature	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_FillColourVar	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_FirstItemVar	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_HighlightStatusVar	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_Index	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_InFileName	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_InteractionStatusVar	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_ItemIndex	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_ItemRefVar	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_ItemStatusVar	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_ItemsToScroll	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_LabelVar	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_LastAnchorFiredVar	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_LineColourVar	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_LineStyleVar	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_LineWidthVar	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_MovementIdentifier	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_NbOfSteps	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_NewCachePriority	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_NewCounterEndPosition	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_NewCounterPosition	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_NewCounterValue	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_NewCursorShape	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_NewEntryPoint	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_NewFirstItem	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_NewHighlightStatus	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_NewIncludedContent	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_NewInteractionStatus	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_NewLabel	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_NewLineStyle	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_NewLineWidth	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_NewOverwriteMode	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_NewPaletteRef	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_NewPortion	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_NewSliderValue	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_NewSpeed	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_NewTransparency	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_NewVolume	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_NewXPosition	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_NewYPosition	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_Numerator	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_OpenSucceeded	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_Operator	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_OutFileName	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_OverwriteModeVar	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_PortionVar	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_Protocol	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_ReadSucceeded	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_ReferenceVisible	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_RunningStatusVar	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_SelectionStatusVar	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_SizeVar	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_SliderValueVar	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_StartAngle	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_StoreSucceeded	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_Target	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_TextContentVar	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_TextDataVar	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_TimerID	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_TimerValue	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_TokenPositionVar	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_TransitionEffect	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_TriggerIdentifier	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_Value	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_VisibleReference	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_VolumeVar	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_X	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_X1	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_X2	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_XBoxSizeVar	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_XCursor	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_XNewBoxSize	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_XOut	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_XPositionVar	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_XScale	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_Y	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_Y1	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_Y2	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_YBoxSizeVar	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_YCursor	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_YNewBoxSize	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_YOut	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_YPositionVar	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_YScale	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_XOffsetVar	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_YOffsetVar	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_NewXOffset	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_NewYOffset	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_FocusPositionVar	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_NewFocusPosition	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_NewMinValue	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_NewMaxValue	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_NewStepSize	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_InternalReference	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_GroupIdentifier	ASN1TAGCLASS_OctetString
#define ASN1TAGCLASS_ObjectNumber	ASN1TAGCLASS_INTEGER
#define ASN1TAGCLASS_DirectReference	ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_ColourIndex	ASN1TAGCLASS_INTEGER
#define ASN1TAGCLASS_AbsoluteColour	ASN1TAGCLASS_OctetString
#define ASN1TAGCLASS_XPosition	ASN1TAGCLASS_INTEGER
#define ASN1TAGCLASS_YPosition	ASN1TAGCLASS_INTEGER
#define ASN1TAGCLASS_ReferencedContent ASN1TAGCLASS_SEQUENCE
#define ASN1TAGCLASS_XYPosition  ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_Point  ASN1TAGCLASS_SEQUENCE
#define ASN1TAGCLASS_Rational   ASN1TAGCLASS_SEQUENCE
#define ASN1TAGCLASS_ExternalReference  ASN1TAGCLASS_SEQUENCE
#define ASN1TAGCLASS_NewReferencedContent ASN1TAGCLASS_SEQUENCE
#define ASN1TAGCLASS_NextScene   ASN1TAGCLASS_SEQUENCE
#define ASN1TAGCLASS_TokenGroupItem  ASN1TAGCLASS_SEQUENCE
#define ASN1TAGCLASS_Movement		ASN1TAGCLASS_SEQUENCE
#define ASN1TAGCLASS_ActionSlots ASN1TAGCLASS_SEQUENCE
#define ASN1TAGCLASS_InVariables ASN1TAGCLASS_SEQUENCE
#define ASN1TAGCLASS_OutVariables ASN1TAGCLASS_SEQUENCE
#define ASN1TAGCLASS_ActionClass ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_Parameters  ASN1TAGCLASS_SEQUENCE
#define ASN1TAGCLASS_PointList  ASN1TAGCLASS_SEQUENCE
/* added by me to get an explicit SEQUENCE under NoTokenActionSlot */
#define ASN1TAGCLASS_ActionClassSeq	ASN1TAGCLASS_SEQUENCE
/* ContentReference is [69] except in ReferencedContent */
#define ASN1TAGCLASS_ContentReference	ASN1TAG_SYNTHETIC

/* needed by mhegd */
#define ASN1TAGCLASS_InterchangedObject		ASN1TAG_SYNTHETIC
#define ASN1TAGCLASS_ActionSlot			ASN1TAG_CHOICE
/* TODO: check this, also make sure NoTokenActionSlots are decoded correctly */
#define ASN1TAGCLASS_ActionSlotSeq		ASN1TAGCLASS_SEQUENCE
#define ASN1TAGCLASS_ButtonStyleEnum		ASN1TAGCLASS_ENUMERATED
#define ASN1TAGCLASS_Colour			ASN1TAG_CHOICE
#define ASN1TAGCLASS_ContentBody		ASN1TAG_CHOICE
#define ASN1TAGCLASS_DefaultAttribute		ASN1TAG_CHOICE
#define ASN1TAGCLASS_ElementaryAction		ASN1TAG_CHOICE
#define ASN1TAGCLASS_EventDataBody		ASN1TAG_CHOICE
#define ASN1TAGCLASS_EventTypeEnum		ASN1TAGCLASS_ENUMERATED
#define ASN1TAGCLASS_FontBody			ASN1TAG_CHOICE
#define ASN1TAGCLASS_GenericBoolean		ASN1TAG_CHOICE
#define ASN1TAGCLASS_GenericContentReference	ASN1TAG_CHOICE
#define ASN1TAGCLASS_GenericInteger		ASN1TAG_CHOICE
#define ASN1TAGCLASS_GenericObjectReference	ASN1TAG_CHOICE
#define ASN1TAGCLASS_GenericOctetString		ASN1TAG_CHOICE
#define ASN1TAGCLASS_GroupItem			ASN1TAG_CHOICE
#define ASN1TAGCLASS_InputTypeEnum		ASN1TAGCLASS_ENUMERATED
#define ASN1TAGCLASS_JustificationEnum		ASN1TAGCLASS_ENUMERATED
#define ASN1TAGCLASS_LineOrientationEnum	ASN1TAGCLASS_ENUMERATED
#define ASN1TAGCLASS_OrientationEnum		ASN1TAGCLASS_ENUMERATED
#define ASN1TAGCLASS_OriginalValueBody		ASN1TAG_CHOICE
#define ASN1TAGCLASS_Parameter			ASN1TAG_CHOICE
#define ASN1TAGCLASS_SliderStyleEnum		ASN1TAGCLASS_ENUMERATED
#define ASN1TAGCLASS_StartCornerEnum		ASN1TAGCLASS_ENUMERATED
#define ASN1TAGCLASS_StorageEnum		ASN1TAGCLASS_ENUMERATED
#define ASN1TAGCLASS_StreamComponent		ASN1TAG_CHOICE
#define ASN1TAGCLASS_TerminationEnum		ASN1TAGCLASS_ENUMERATED
#define ASN1TAGCLASS_ComparisonValue		ASN1TAG_CHOICE
#define ASN1TAGCLASS_EmulatedEventData		ASN1TAG_CHOICE
#define ASN1TAGCLASS_NewColour			ASN1TAG_CHOICE
#define ASN1TAGCLASS_NewContent			ASN1TAG_CHOICE
#define ASN1TAGCLASS_NewFont			ASN1TAG_CHOICE
#define ASN1TAGCLASS_NewVariableValue		ASN1TAG_CHOICE
#define ASN1TAGCLASS_NewTimer			ASN1TAGCLASS_SEQUENCE

#endif	/* __ASN1TAG_H__ */
