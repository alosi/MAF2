/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: mmgLocaleSettings.cpp,v $
Language:  C++
Date:      $Date: 2006-06-14 14:46:33 $
Version:   $Revision: 1.3 $
Authors:   Paolo Quadrani
==========================================================================
Copyright (c) 2001/2005 
CINECA - Interuniversity Consortium (www.cineca.it)
=========================================================================*/


#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "mmgLocaleSettings.h"
#include <wx/intl.h>
#include <wx/config.h>
#include "mmgGui.h"
#include "mmgDialog.h"

//----------------------------------------------------------------------------
mmgLocaleSettings::mmgLocaleSettings(mafObserver *Listener)
//----------------------------------------------------------------------------
{
	m_Listener   = Listener;
  m_LanguageId = 0;
  InitializeLanguage();
  
  //SIL. 09-jun-2006 :
  wxString lang_array[5] = {"English","French","German","Italian","Spanish"};

  m_Gui = new mmgGui(this);   
  m_Gui->Label("User Interface Language");
  m_Gui->Radio(LANGUAGE_ID,"", &m_LanguageId,5,lang_array);
  m_Gui->Label("changes will take effect when the application restart");
}
//----------------------------------------------------------------------------
mmgLocaleSettings::~mmgLocaleSettings() 
//----------------------------------------------------------------------------
{
}
//----------------------------------------------------------------------------
void mmgLocaleSettings::OnEvent(mafEventBase *maf_event)
//----------------------------------------------------------------------------
{
	switch(maf_event->GetId())
  {
    case LANGUAGE_ID:
    {
      switch(m_LanguageId) 
      {
      case 1:
        m_Language = wxLANGUAGE_FRENCH;
        m_LanguageDictionary = "fr";
        break;
      case 2:
        m_Language = wxLANGUAGE_GERMAN;
        m_LanguageDictionary = "de";
        break;
      case 3:
        m_Language = wxLANGUAGE_ITALIAN;
        m_LanguageDictionary = "it";
        break;
      case 4:
        m_Language = wxLANGUAGE_SPANISH;
        m_LanguageDictionary = "es";
        break;
      default:
        m_Language = wxLANGUAGE_ENGLISH;
        m_LanguageDictionary = "en";
      }
      wxConfig *config = new wxConfig(wxEmptyString);
      config->Write("Language",m_Language);
      config->Write("Dictionary",m_LanguageDictionary);
      cppDEL(config);
    }
    break;
    default:
      mafEventMacro(*maf_event);
    break; 
  }
}
//----------------------------------------------------------------------------
void mmgLocaleSettings::InitializeLanguage()
//----------------------------------------------------------------------------
{
  wxConfig *config = new wxConfig(wxEmptyString);
  long lang;
  wxString dict;
  if(config->Read("Language", &lang))
  {
    m_Language = (wxLanguage)lang;
    config->Read("Dictionary", &dict);
    m_LanguageDictionary = dict;
  }
  else
  {
    // no language set; use default language: English
    config->Write("Language",wxLANGUAGE_ENGLISH);
    config->Write("Dictionary","en");
    m_Language = wxLANGUAGE_ENGLISH;
    m_LanguageDictionary = "en";
  }
  cppDEL(config);

  wxString prefix;
  prefix = wxGetWorkingDirectory();
  prefix += "\\Language\\";
  m_Locale.Init(m_Language);
  m_Locale.AddCatalogLookupPathPrefix(prefix);
  m_Locale.AddCatalog(wxT(m_LanguageDictionary.GetCStr()));
#ifndef WIN32
  m_Locale.AddCatalog("fileutils");
#endif

  switch(m_Language) 
  {
  case wxLANGUAGE_FRENCH:
    m_LanguageId =1;
    break;
  case wxLANGUAGE_GERMAN:
    m_LanguageId =2;
    break;
  case wxLANGUAGE_ITALIAN:
    m_LanguageId =3;
    break;
  case wxLANGUAGE_SPANISH:
    m_LanguageId =4;
    break;
  default: //wxLANGUAGE_ENGLISH;
    m_LanguageId =0; 
  }
}