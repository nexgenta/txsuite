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
 *  Revision      : $Revision: 1386 $
 *  Date          : $Date: 2007-10-30 14:41:17 +0000 (Tue, 30 Oct 2007) $
 *  Author        : $Author: pthomas $
 *  Created By    : Paul Thomas
 *  Create Date   : Mon Jun 9 2003
 *
 *  Description 
 *               FLEX definition file for mhegenc.
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
#include <ctype.h>
    
#include "asn-incl.h"
#include "mh_snacc.h"
#include "mheg.tab.h"
#include "lexer.h"
#include "diag.h"


/* Globals. */
int token_start_offset=0;
int token_end_offset=0;
char line_buf[LINE_BUFFER_LENGTH];
YYLTYPE yylloc;
YYSTYPE yylval;

/* Locals. */
static const char *rcsid =
    "@(#) $Id: mheg.lex 1386 2007-10-30 14:41:17Z pthomas $";
static int first_run = 1;

static void X(void);

%}

%option caseless
%option yylineno
%option noyywrap

delim       [\x09\x0a\x0c\x0d\x20]
ws          [\x09\x20]+
nl          [\x0a\x0c\x0d]
notnl       [^\x0a\x0c\x0d]
LPAREN      \x28
RPAREN      \x29
LBRACE      \x7b
RBRACE      \x7d
WD          [\x09\x0a\x0c\x0d\x20\x28\x29\x7b\x7d]
WORDCHAR    [^\x09\x0a\x0c\x0d\x20\x28\x29\x7b\x7d]
DIGIT       [\x31-\x39]
DIGIT0      {DIGIT}|\x30
HEXPREFIX   \x30\x78|\x30\x58
HEXDIGIT0   [\x61-\x66\x41-\x46\x30-\x39]
HEXINT      {HEXPREFIX}{HEXDIGIT0}+
DECINT      \x2d?{DIGIT}{DIGIT0}*
INTEGER     {DECINT}|{HEXINT}|\x30

TRUE        true
FALSE       false

ALPHACHAR   [\x41-\x5a\x61-\x7a]
PRINTABLE   [\x20-\x7e]
STRINGCHAR  [\x20-\x21\x23-\x5b\x5d-\x7e]|(\x5c\x22)|(\x5c\x5c)
STRING      \x22{STRINGCHAR}*\x22

QPRINTABLE  \x27[^\x27]*\x27

BASE64      \x60[^\x60]*\x60

NULL        null

ENUMERATION {ALPHACHAR}{WORDCHAR}*

colon       \x3a


%x FIRST
%%
    if( first_run ){
        BEGIN(FIRST);
        first_run = 0;
    }

<FIRST>.*       {token_end_offset=0;
                 strncpy(line_buf, yytext, LINE_BUFFER_LENGTH);
                 yyless(0);
                 BEGIN(0);}
\n.*            {token_end_offset=0;
                 strncpy(line_buf, yytext+1, LINE_BUFFER_LENGTH);
                 yyless(1);}

{ws}            {X(); continue; /* Eat Whitespace. */}
"//"{notnl}*    {X(); continue; /* Ignore comments. */}

{NULL}/{WD}     {X(); return eTOK_NULL;}


{INTEGER}/{WD}  {X(); yylval.ival = atoi(yytext); return eTOK_INTEGER;}

{TRUE}/{WD}     {X(); yylval.ival = 1;return eTOK_BOOLEAN;}
{FALSE}/{WD}    {X(); yylval.ival = 0;return eTOK_BOOLEAN;}

{STRING}/{WD}   {X(); yylval.sval.octs = yytext;
                      yylval.sval.octetLen = yyleng; return eTOK_STRING;}
{QPRINTABLE}/{WD} {X(); yylval.sval.octs = yytext; 
                      yylval.sval.octetLen = yyleng; return eTOK_STRING;}
{BASE64}/{WD}   {X(); yylval.sval.octs = yytext; 
                      yylval.sval.octetLen = yyleng; return eTOK_STRING;}

{LPAREN}        {X();  return eTOK_LPAREN; }
{RPAREN}        {X();  return eTOK_RPAREN; }
{LBRACE}        {X();  return eTOK_LBRACE; }
{RBRACE}        {X();  return eTOK_RBRACE; }

"=="/{WD}       {X(); return eTOK_EXT_Equals; }
"!="/{WD}       {X(); return eTOK_EXT_NotEquals; }
"<"/{WD}        {X(); return eTOK_EXT_StrictlyLess; }
"<="/{WD}       {X(); return eTOK_EXT_LessOrEqual; }
">"/{WD}        {X(); return eTOK_EXT_StrictlyGreater; }
">="/{WD}       {X(); return eTOK_EXT_GreaterOrEqual; }

"+"/{WD}        {X(); return eTOK_EXT_Plus;}
"-"/{WD}        {X(); return eTOK_EXT_Minus;}
"*"/{WD}        {X(); return eTOK_EXT_Multiply;}
"/"/{WD}        {X(); return eTOK_EXT_Divide;}

:StdVersion/{WD}            {X(); return eTOK_TAG_StandardVersion;}
:StdID/{WD}                 {X(); return eTOK_TAG_StandardIdentifier;}
:ObjectInfo/{WD}            {X(); return eTOK_TAG_ObjectInformation;}
:OnStartUp/{WD}             {X(); return eTOK_TAG_OnStartUp;}
:OnCloseDown/{WD}           {X(); return eTOK_TAG_OnCloseDown;}
:Items/{WD}                 {X(); return eTOK_TAG_Items;}
:OrigGCPriority/{WD}        {X(); return eTOK_TAG_OriginalGroupCachePriority;}
:OnSpawnCloseDown/{WD}      {X(); return eTOK_TAG_OnSpawnCloseDown;}
:OnRestart/{WD}             {X(); return eTOK_TAG_OnRestart;}
:CharacterSet/{WD}          {X(); return eTOK_TAG_CharacterSet;}
:BackgroundColour/{WD}      {X(); return eTOK_TAG_BackgroundColour;}
:TextCHook/{WD}             {X(); return eTOK_TAG_TextContentHook;}
:TextColour/{WD}            {X(); return eTOK_TAG_TextColour;}
:FontAttributes/{WD}        {X(); return eTOK_TAG_FontAttributes;}
:Font/{WD}                  {X(); return eTOK_TAG_Font;}
:IProgramCHook/{WD}         {X(); return eTOK_TAG_IProgramContentHook;}
:StreamCHook/{WD}           {X(); return eTOK_TAG_StreamContentHook;}
:BitmapCHook/{WD}           {X(); return eTOK_TAG_BitmapContentHook;}
:LineArtCHook/{WD}          {X(); return eTOK_TAG_LineArtContentHook;}
:ButtonRefColour/{WD}       {X(); return eTOK_TAG_ButtonRefColour;}
:HighlightRefColour/{WD}    {X(); return eTOK_TAG_HighlightRefColour;}
:SliderRefColour/{WD}       {X(); return eTOK_TAG_SliderRefColour;}
:OrigDesktopColour/{WD}     {X(); return eTOK_TAG_OriginalDesktopColour;}
:InputEventReg/{WD}         {X(); return eTOK_TAG_InputEventRegister;}
:SceneCS/{WD}               {X(); return eTOK_TAG_SceneCoordinateSystem;}
:AspectRatio/{WD}           {X(); return eTOK_TAG_AspectRatio;}
:MovingCursor/{WD}          {X(); return eTOK_TAG_MovingCursor;}
:NextScenes/{WD}            {X(); return eTOK_TAG_NextScenes;}
:InitiallyActive/{WD}       {X(); return eTOK_TAG_InitiallyActive;}
:CHook/{WD}                 {X(); return eTOK_TAG_ContentHook;}
:OrigContent/{WD}           {X(); return eTOK_TAG_OriginalContent;}
:ContentRef/{WD}            {X(); return eTOK_TAG_ContentRef;}
:ContentSize/{WD}           {X(); return eTOK_TAG_ContentSize;}
:CCPriority/{WD}            {X(); return eTOK_TAG_ContentCachePriority;}
:Shared/{WD}                {X(); return eTOK_TAG_Shared;}
:EventSource/{WD}           {X(); return eTOK_TAG_EventSource;}
:EventType/{WD}             {X(); return eTOK_TAG_EventType;}
:EventData/{WD}             {X(); return eTOK_TAG_EventData;}
:LinkEffect/{WD}            {X(); return eTOK_TAG_LinkEffect;}
:Name/{WD}                  {X(); return eTOK_TAG_Name;}
:InitiallyAvailable/{WD}    {X(); return eTOK_TAG_InitiallyAvailable;}
:OrigValue/{WD}             {X(); return eTOK_TAG_OriginalValue;}
:ObjectRef/{WD}             {X(); return eTOK_TAG_ObjectReferenceValue;}
:MovementTable/{WD}         {X(); return eTOK_TAG_MovementTable;}
:TokenGroupItems/{WD}       {X(); return eTOK_TAG_TokenGroupItems;}
:NoTokenActionSlots/{WD}    {X(); return eTOK_TAG_NoTokenActionSlots;}
:Positions/{WD}             {X(); return eTOK_TAG_Positions;}
:ActionSlots/{WD}           {X(); return eTOK_TAG_ActionSlots;}
:WrapAround/{WD}            {X(); return eTOK_TAG_WrapAround;}
:MultipleSelection/{WD}     {X(); return eTOK_TAG_MultipleSelection;}
:OrigBoxSize/{WD}           {X(); return eTOK_TAG_OriginalBoxSize;}
:OrigPosition/{WD}          {X(); return eTOK_TAG_OriginalPosition;}
:OrigPaletteRef/{WD}        {X(); return eTOK_TAG_OriginalPaletteRef;}
:Tiling/{WD}                {X(); return eTOK_TAG_Tiling;}
:OrigTransparency/{WD}      {X(); return eTOK_TAG_OriginalTransparency;}
:BBBox/{WD}                 {X(); return eTOK_TAG_BorderedBoundingBox;}
:OrigLineWidth/{WD}         {X(); return eTOK_TAG_OriginalLineWidth;}
:OrigLineStyle/{WD}         {X(); return eTOK_TAG_OriginalLineStyle;}
:OrigRefFillColour/{WD}     {X(); return eTOK_TAG_OriginalRefFillColour;}
:OrigRefLineColour/{WD}     {X(); return eTOK_TAG_OriginalRefLineColour;}
:OrigFont/{WD}              {X(); return eTOK_TAG_OriginalFont;}
:HJustification/{WD}        {X(); return eTOK_TAG_HorizontalJustification;}
:VJustification/{WD}        {X(); return eTOK_TAG_VerticalJustification;}
:LineOrientation/{WD}       {X(); return eTOK_TAG_LineOrientation;}
:StartCorner/{WD}           {X(); return eTOK_TAG_StartCorner;}
:TextWrapping/{WD}          {X(); return eTOK_TAG_TextWrapping;}
:Multiplex/{WD}             {X(); return eTOK_TAG_Multiplex;}
:Storage/{WD}               {X(); return eTOK_TAG_Storage;}
:Looping/{WD}               {X(); return eTOK_TAG_Looping;}
:ComponentTag/{WD}          {X(); return eTOK_TAG_ComponentTag;}
:OrigVolume/{WD}            {X(); return eTOK_TAG_OriginalVolume;}
:Termination/{WD}           {X(); return eTOK_TAG_Termination;}
:EngineResp/{WD}            {X(); return eTOK_TAG_EngineResp;}
:Orientation/{WD}           {X(); return eTOK_TAG_Orientation;}
:MaxValue/{WD}              {X(); return eTOK_TAG_MaxValue;}
:MinValue/{WD}              {X(); return eTOK_TAG_MinValue;}
:InitialValue/{WD}          {X(); return eTOK_TAG_InitialValue;}
:InitialPortion/{WD}        {X(); return eTOK_TAG_InitialPortion;}
:InitialPosition/{WD}       {X(); return eTOK_TAG_InitialPosition;}
:StepSize/{WD}              {X(); return eTOK_TAG_StepSize;}
:SliderStyle/{WD}           {X(); return eTOK_TAG_SliderStyle;}
:InputType/{WD}             {X(); return eTOK_TAG_InputType;}
:CharList/{WD}              {X(); return eTOK_TAG_CharList;}
:ObscuredInput/{WD}         {X(); return eTOK_TAG_ObscuredInput;}
:MaxLength/{WD}             {X(); return eTOK_TAG_MaxLength;}
:OrigLabel/{WD}             {X(); return eTOK_TAG_OriginalLabel;}
:ButtonStyle/{WD}           {X(); return eTOK_TAG_ButtonStyle;}
:Activate/{WD}              {X(); return eTOK_TAG_Activate;}
:Add/{WD}                   {X(); return eTOK_TAG_Add;}
:AddItem/{WD}               {X(); return eTOK_TAG_AddItem;}
:Append/{WD}                {X(); return eTOK_TAG_Append;}
:BringToFront/{WD}          {X(); return eTOK_TAG_BringToFront;}
:Call/{WD}                  {X(); return eTOK_TAG_Call;}
:CallActionSlot/{WD}        {X(); return eTOK_TAG_CallActionSlot;}
:Clear/{WD}                 {X(); return eTOK_TAG_Clear;}
:Clone/{WD}                 {X(); return eTOK_TAG_Clone;}
:CloseConnection/{WD}       {X(); return eTOK_TAG_CloseConnection;}
:Deactivate/{WD}            {X(); return eTOK_TAG_Deactivate;}
:DelItem/{WD}               {X(); return eTOK_TAG_DelItem;}
:Deselect/{WD}              {X(); return eTOK_TAG_Deselect;}
:DeselectItem/{WD}          {X(); return eTOK_TAG_DeselectItem;}
:Divide/{WD}                {X(); return eTOK_TAG_Divide;}
:DrawArc/{WD}               {X(); return eTOK_TAG_DrawArc;}
:DrawLine/{WD}              {X(); return eTOK_TAG_DrawLine;}
:DrawOval/{WD}              {X(); return eTOK_TAG_DrawOval;}
:DrawPolygon/{WD}           {X(); return eTOK_TAG_DrawPolygon;}
:DrawPolyline/{WD}          {X(); return eTOK_TAG_DrawPolyline;}
:DrawRectangle/{WD}         {X(); return eTOK_TAG_DrawRectangle;}
:DrawSector/{WD}            {X(); return eTOK_TAG_DrawSector;}
:Fork/{WD}                  {X(); return eTOK_TAG_Fork;}
:GetAvailabilityStatus/{WD} {X(); return eTOK_TAG_GetAvailabilityStatus;}
:GetBoxSize/{WD}            {X(); return eTOK_TAG_GetBoxSize;}
:GetCellItem/{WD}           {X(); return eTOK_TAG_GetCellItem;}
:GetCursorPosition/{WD}     {X(); return eTOK_TAG_GetCursorPosition;}
:GetEngineSupport/{WD}      {X(); return eTOK_TAG_GetEngineSupport;}
:GetEntryPoint/{WD}         {X(); return eTOK_TAG_GetEntryPoint;}
:GetFillColour/{WD}         {X(); return eTOK_TAG_GetFillColour;}
:GetFirstItem/{WD}          {X(); return eTOK_TAG_GetFirstItem;}
:GetHighlightStatus/{WD}    {X(); return eTOK_TAG_GetHighlightStatus;}
:GetInteractionStatus/{WD}  {X(); return eTOK_TAG_GetInteractionStatus;}
:GetItemStatus/{WD}         {X(); return eTOK_TAG_GetItemStatus;}
:GetLabel/{WD}              {X(); return eTOK_TAG_GetLabel;}
:GetLastAnchorFired/{WD}    {X(); return eTOK_TAG_GetLastAnchorFired;}
:GetLineColour/{WD}         {X(); return eTOK_TAG_GetLineColour;}
:GetLineStyle/{WD}          {X(); return eTOK_TAG_GetLineStyle;}
:GetLineWidth/{WD}          {X(); return eTOK_TAG_GetLineWidth;}
:GetListItem/{WD}           {X(); return eTOK_TAG_GetListItem;}
:GetListSize/{WD}           {X(); return eTOK_TAG_GetListSize;}
:GetOverwriteMode/{WD}      {X(); return eTOK_TAG_GetOverwriteMode;}
:GetPortion/{WD}            {X(); return eTOK_TAG_GetPortion;}
:GetPosition/{WD}           {X(); return eTOK_TAG_GetPosition;}
:GetRunningStatus/{WD}      {X(); return eTOK_TAG_GetRunningStatus;}
:GetSelectionStatus/{WD}    {X(); return eTOK_TAG_GetSelectionStatus;}
:GetSliderValue/{WD}        {X(); return eTOK_TAG_GetSliderValue;}
:GetTextContent/{WD}        {X(); return eTOK_TAG_GetTextContent;}
:GetTextData/{WD}           {X(); return eTOK_TAG_GetTextData;}
:GetTokenPosition/{WD}      {X(); return eTOK_TAG_GetTokenPosition;}
:GetVolume/{WD}             {X(); return eTOK_TAG_GetVolume;}
:Launch/{WD}                {X(); return eTOK_TAG_Launch;}
:LockScreen/{WD}            {X(); return eTOK_TAG_LockScreen;}
:Modulo/{WD}                {X(); return eTOK_TAG_Modulo;}
:Move/{WD}                  {X(); return eTOK_TAG_Move;}
:MoveTo/{WD}                {X(); return eTOK_TAG_MoveTo;}
:Multiply/{WD}              {X(); return eTOK_TAG_Multiply;}
:OpenConnection/{WD}        {X(); return eTOK_TAG_OpenConnection;}
:Preload/{WD}               {X(); return eTOK_TAG_Preload;}
:PutBefore/{WD}             {X(); return eTOK_TAG_PutBefore;}
:PutBehind/{WD}             {X(); return eTOK_TAG_PutBehind;}
:Quit/{WD}                  {X(); return eTOK_TAG_Quit;}
:ReadPersistent/{WD}        {X(); return eTOK_TAG_ReadPersistent;}
:Run/{WD}                   {X(); return eTOK_TAG_Run;}
:ScaleBitmap/{WD}           {X(); return eTOK_TAG_ScaleBitmap;}
:ScaleVideo/{WD}            {X(); return eTOK_TAG_ScaleVideo;}
:ScrollItems/{WD}           {X(); return eTOK_TAG_ScrollItems;}
:Select/{WD}                {X(); return eTOK_TAG_Select;}
:SelectItem/{WD}            {X(); return eTOK_TAG_SelectItem;}
:SendEvent/{WD}             {X(); return eTOK_TAG_SendEvent;}
:SendToBack/{WD}            {X(); return eTOK_TAG_SendToBack;}
:SetBoxSize/{WD}            {X(); return eTOK_TAG_SetBoxSize;}
:SetCachePriority/{WD}      {X(); return eTOK_TAG_SetCachePriority;}
:SetCounterEndPosition/{WD} {X(); return eTOK_TAG_SetCounterEndPosition;}
:SetCounterPosition/{WD}    {X(); return eTOK_TAG_SetCounterPosition;}
:SetCounterTrigger/{WD}     {X(); return eTOK_TAG_SetCounterTrigger;}
:SetCursorPosition/{WD}     {X(); return eTOK_TAG_SetCursorPosition;}
:SetCursorShape/{WD}        {X(); return eTOK_TAG_SetCursorShape;}
:SetData/{WD}               {X(); return eTOK_TAG_SetData;}
:SetEntryPoint/{WD}         {X(); return eTOK_TAG_SetEntryPoint;}
:SetFillColour/{WD}         {X(); return eTOK_TAG_SetFillColour;}
:SetFirstItem/{WD}          {X(); return eTOK_TAG_SetFirstItem;}
:SetFontRef/{WD}            {X(); return eTOK_TAG_SetFontRef;}
:SetHighlightStatus/{WD}    {X(); return eTOK_TAG_SetHighlightStatus;}
:SetInteractionStatus/{WD}  {X(); return eTOK_TAG_SetInteractionStatus;}
:SetLabel/{WD}              {X(); return eTOK_TAG_SetLabel;}
:SetLineColour/{WD}         {X(); return eTOK_TAG_SetLineColour;}
:SetLineStyle/{WD}          {X(); return eTOK_TAG_SetLineStyle;}
:SetLineWidth/{WD}          {X(); return eTOK_TAG_SetLineWidth;}
:SetOverwriteMode/{WD}      {X(); return eTOK_TAG_SetOverwriteMode;}
:SetPaletteRef/{WD}         {X(); return eTOK_TAG_SetPaletteRef;}
:SetPortion/{WD}            {X(); return eTOK_TAG_SetPortion;}
:SetPosition/{WD}           {X(); return eTOK_TAG_SetPosition;}
:SetSliderValue/{WD}        {X(); return eTOK_TAG_SetSliderValue;}
:SetSpeed/{WD}              {X(); return eTOK_TAG_SetSpeed;}
:SetTimer/{WD}              {X(); return eTOK_TAG_SetTimer;}
:SetTransparency/{WD}       {X(); return eTOK_TAG_SetTransparency;}
:SetVariable/{WD}           {X(); return eTOK_TAG_SetVariable;}
:SetVolume/{WD}             {X(); return eTOK_TAG_SetVolume;}
:Spawn/{WD}                 {X(); return eTOK_TAG_Spawn;}
:Step/{WD}                  {X(); return eTOK_TAG_Step;}
:Stop/{WD}                  {X(); return eTOK_TAG_Stop;}
:StorePersistent/{WD}       {X(); return eTOK_TAG_StorePersistent;}
:Subtract/{WD}              {X(); return eTOK_TAG_Subtract;}
:TestVariable/{WD}          {X(); return eTOK_TAG_TestVariable;}
:Toggle/{WD}                {X(); return eTOK_TAG_Toggle;}
:ToggleItem/{WD}            {X(); return eTOK_TAG_ToggleItem;}
:TransitionTo/{WD}          {X(); return eTOK_TAG_TransitionTo;}
:Unload/{WD}                {X(); return eTOK_TAG_Unload;}
:UnlockScreen/{WD}          {X(); return eTOK_TAG_UnlockScreen;}
:SetBackgroundColour/{WD}   {X(); return eTOK_TAG_SetBackgroundColour;}
:SetTextColour/{WD}         {X(); return eTOK_TAG_SetTextColour;}
:SetFontAttributes/{WD}     {X(); return eTOK_TAG_SetFontAttributes;}
:SetCellPosition/{WD}       {X(); return eTOK_TAG_SetCellPosition;}
:SetInputReg/{WD}           {X(); return eTOK_TAG_SetInputRegister;}
:SetBitmapDecodeOffset/{WD} {X(); return eTOK_TAG_SetBitmapDecodeOffset;}
:GetBitmapDecodeOffset/{WD} {X(); return eTOK_TAG_GetBitmapDecodeOffset;}
:SetVideoDecodeOffset/{WD}  {X(); return eTOK_TAG_SetVideoDecodeOffset;}
:GetVideoDecodeOffset/{WD}  {X(); return eTOK_TAG_GetVideoDecodeOffset;}
:SetFocusPosition/{WD}      {X(); return eTOK_TAG_SetFocusPosition;}
:GetFocusPosition/{WD}      {X(); return eTOK_TAG_GetFocusPosition;}
:SetSliderParameters/{WD}   {X(); return eTOK_TAG_SetSliderParameters;}
:SetDesktopColour/{WD}      {X(); return eTOK_TAG_SetDesktopColour;}

:IndirectRef/{WD}           {X(); return eTOK_TAG_IndirectReference;}
:GBoolean/{WD}              {X(); return eTOK_TAG_GenericBoolean;}
:GInteger/{WD}              {X(); return eTOK_TAG_GenericInteger;}
:GOctetString/{WD}          {X(); return eTOK_TAG_GenericOctetString;}
:GObjectRef/{WD}            {X(); return eTOK_TAG_GenericObjectReference;}
:GContentRef/{WD}           {X(); return eTOK_TAG_GenericContentReference;}
:ConnectionTag/{WD}         {X(); return eTOK_TAG_ConnectionTag;}
:NewContentSize/{WD}        {X(); return eTOK_TAG_NewContentSize;}
:NewCCPriority/{WD}         {X(); return eTOK_TAG_NewContentCachePriority;}
:NewRefContent/{WD}         {X(); return eTOK_TAG_NewReferencedContent;}
:NewColourIndex/{WD}        {X(); return eTOK_TAG_NewColourIndex;}
:NewAbsoluteColour/{WD}     {X(); return eTOK_TAG_NewAbsoluteColour;}
:AbsoluteTime/{WD}          {X(); return eTOK_TAG_AbsoluteTime;}
    
:Application/{WD}           {X(); return eTOK_TAG_Application;}
:Scene/{WD}                 {X(); return eTOK_TAG_Scene;}
:Link/{WD}                  {X(); return eTOK_TAG_Link;}
:ResidentPrg/{WD}           {X(); return eTOK_TAG_ResidentProgram;}
:RemotePrg/{WD}             {X(); return eTOK_TAG_RemoteProgram;}
:InterchgPrg/{WD}           {X(); return eTOK_TAG_InterchangedProgram;}
:Palette/{WD}               {X(); return eTOK_TAG_Palette;}
:CursorShape/{WD}           {X(); return eTOK_TAG_CursorShape;}
:BooleanVar/{WD}            {X(); return eTOK_TAG_BooleanVariable;}
:IntegerVar/{WD}            {X(); return eTOK_TAG_IntegerVariable;}
:OStringVar/{WD}            {X(); return eTOK_TAG_OctetStringVariable;}
:ObjectRefVar/{WD}          {X(); return eTOK_TAG_ObjectRefVariable;}
:ContentRefVar/{WD}         {X(); return eTOK_TAG_ContentRefVariable;}
:TokenGroup/{WD}            {X(); return eTOK_TAG_TokenGroup;}
:ListGroup/{WD}             {X(); return eTOK_TAG_ListGroup;}
:Bitmap/{WD}                {X(); return eTOK_TAG_Bitmap;}
:LineArt/{WD}               {X(); return eTOK_TAG_LineArt;}
:Rectangle/{WD}             {X(); return eTOK_TAG_Rectangle;}
:DynamicLineArt/{WD}        {X(); return eTOK_TAG_DynamicLineArt;}
:Text/{WD}                  {X(); return eTOK_TAG_Text;}
:Stream/{WD}                {X(); return eTOK_TAG_Stream;}
:Audio/{WD}                 {X(); return eTOK_TAG_Audio;}
:Video/{WD}                 {X(); return eTOK_TAG_Video;}
:RTGraphics/{WD}            {X(); return eTOK_TAG_RTGraphics;}
:Slider/{WD}                {X(); return eTOK_TAG_Slider;}
:EntryField/{WD}            {X(); return eTOK_TAG_EntryField;}
:HyperText/{WD}             {X(); return eTOK_TAG_HyperText;}
:Hotspot/{WD}               {X(); return eTOK_TAG_Hotspot;}
:PushButton/{WD}            {X(); return eTOK_TAG_PushButton;}
:SwitchButton/{WD}          {X(); return eTOK_TAG_SwitchButton;}

IsAvailable/{WD}            {X(); return eTOK_ENUM_IsAvailable;}
ContentAvailable/{WD}       {X(); return eTOK_ENUM_ContentAvailable;}
IsDeleted/{WD}              {X(); return eTOK_ENUM_IsDeleted;}
IsRunning/{WD}              {X(); return eTOK_ENUM_IsRunning;}
IsStopped/{WD}              {X(); return eTOK_ENUM_IsStopped;}
UserInput/{WD}              {X(); return eTOK_ENUM_UserInput;}
AnchorFired/{WD}            {X(); return eTOK_ENUM_AnchorFired;}
TimerFired/{WD}             {X(); return eTOK_ENUM_TimerFired;}
AsynchStopped/{WD}          {X(); return eTOK_ENUM_AsynchStopped;}
InteractionCompleted/{WD}   {X();return eTOK_ENUM_InteractionCompleted;}
TokenMovedFrom/{WD}         {X(); return eTOK_ENUM_TokenMovedFrom;}
TokenMovedTo/{WD}           {X(); return eTOK_ENUM_TokenMovedTo;}
StreamEvent/{WD}            {X(); return eTOK_ENUM_StreamEvent;}
StreamPlaying/{WD}          {X(); return eTOK_ENUM_StreamPlaying;}
StreamStopped/{WD}          {X(); return eTOK_ENUM_StreamStopped;}
CounterTrigger/{WD}         {X(); return eTOK_ENUM_CounterTrigger;}
HighlightOn/{WD}            {X(); return eTOK_ENUM_HighlightOn;}
HighlightOff/{WD}           {X(); return eTOK_ENUM_HighlightOff;}
CursorEnter/{WD}            {X(); return eTOK_ENUM_CursorEnter;}
CursorLeave/{WD}            {X(); return eTOK_ENUM_CursorLeave;}
IsSelected/{WD}             {X(); return eTOK_ENUM_IsSelected;}
IsDeselected/{WD}           {X(); return eTOK_ENUM_IsDeselected;}
TestEvent/{WD}              {X(); return eTOK_ENUM_TestEvent;}
FirstItemPresented/{WD}     {X(); return eTOK_ENUM_FirstItemPresented;}
LastItemPresented/{WD}      {X(); return eTOK_ENUM_LastItemPresented;}
HeadItems/{WD}              {X(); return eTOK_ENUM_HeadItems;}
TailItems/{WD}              {X(); return eTOK_ENUM_TailItems;}
ItemSelected/{WD}           {X(); return eTOK_ENUM_ItemSelected;}
ItemDeselected/{WD}         {X(); return eTOK_ENUM_ItemDeselected;}
EntryFieldFull/{WD}         {X(); return eTOK_ENUM_EntryFieldFull;}
EngineEvent/{WD}            {X(); return eTOK_ENUM_EngineEvent;}
FocusMoved/{WD}             {X(); return eTOK_ENUM_FocusMoved;}
SliderValueChanged/{WD}     {X(); return eTOK_ENUM_SliderValueChanged;}

start/{WD}                  {X(); return eTOK_ENUM_start;}
end/{WD}                    {X(); return eTOK_ENUM_end;}
centre/{WD}                 {X(); return eTOK_ENUM_centre;}
justified/{WD}              {X(); return eTOK_ENUM_justified;}
vertical/{WD}               {X(); return eTOK_ENUM_vertical;}
horizontal/{WD}             {X(); return eTOK_ENUM_horizontal;}
upper-left/{WD}             {X(); return eTOK_ENUM_upper_left;}
upper-right/{WD}            {X(); return eTOK_ENUM_upper_right;}
lower-left/{WD}             {X(); return eTOK_ENUM_lower_left;}
lower-right/{WD}            {X(); return eTOK_ENUM_lower_right;}
memory/{WD}                 {X(); return eTOK_ENUM_memory;}
stream/{WD}                 {X(); return eTOK_ENUM_stream;}
freeze/{WD}                 {X(); return eTOK_ENUM_freeze;}
disappear/{WD}              {X(); return eTOK_ENUM_disappear;}
left/{WD}                   {X(); return eTOK_ENUM_left;}
right/{WD}                  {X(); return eTOK_ENUM_right;}
up/{WD}                     {X(); return eTOK_ENUM_up;}
down/{WD}                   {X(); return eTOK_ENUM_down;}
normal/{WD}                 {X(); return eTOK_ENUM_normal;}
thermometer/{WD}            {X(); return eTOK_ENUM_thermometer;}
proportional/{WD}           {X(); return eTOK_ENUM_proportional;}
alpha/{WD}                  {X(); return eTOK_ENUM_alpha;}
numeric/{WD}                {X(); return eTOK_ENUM_numeric;}
any/{WD}                    {X(); return eTOK_ENUM_any;}
listed/{WD}                 {X(); return eTOK_ENUM_listed;}
pushbutton/{WD}             {X(); return eTOK_ENUM_pushbutton;}
radiobutton/{WD}            {X(); return eTOK_ENUM_radiobutton;}
checkbox/{WD}               {X(); return eTOK_ENUM_checkbox;}

#{notnl}*                   {X(); continue; /* Ignore preprocessor. */}

{WORDCHAR}+   {
    char msg_buf[LINE_BUFFER_LENGTH];
    X();

    switch(*yytext)
    {
    case ':':
        sprintf(msg_buf, "Unknown Tag '%s'", yytext);
        break;
    case '\'':
    case '"':
    case '`':
        sprintf(msg_buf, "Bad String");
        break;
    default:
        if( isalpha(*yytext) ){
            sprintf(msg_buf, "Unknown enumeration value %s", yytext);
        }
        else if( isdigit(*yytext) ){
            sprintf(msg_buf, "Bad Number '%s'", yytext);
        }
        else
            sprintf(msg_buf, "Unknown Token '%s'", yytext);
    }
    diag_msg(eSEVERITY_ERROR, msg_buf, yylineno);
    fprintf(stderr, "%s\n", line_buf);
    fprintf(stderr, "%*s\n", 1+ (token_start_offset), "^");
    /* TODO: try to find a match */
}

%%

/* Called at begining of each action. */
static void X(void)
{
    token_start_offset = token_end_offset;
    token_end_offset += yyleng;
    yylloc.first_column = token_start_offset;
    yylloc.first_line = yylineno;
}




/*
 * Local variables:
 * compile-command: "make -C ../"
 *  tab-width: 4
 * End:
 */
