/*=========================================================================

 Program: MAF2
 Module: mafPipeFactory
 Authors: Marco Petrone
 
 Copyright (c) B3C
 All rights reserved. See Copyright.txt or
 http://www.scsitaly.com/Copyright.htm for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#ifndef __mafPipeFactory_h
#define __mafPipeFactory_h
//----------------------------------------------------------------------------
// includes :
//----------------------------------------------------------------------------
#include "mafObjectFactory.h"

/** to be used internally for plugging default pipes --- calls a member function directly */
#define mafPlugPipeMacro(pipe_type,descr) \
  RegisterNewPipe(pipe_type::GetStaticTypeName(), descr, pipe_type::NewObject);
  

//----------------------------------------------------------------------------
// forward declarations :
//----------------------------------------------------------------------------  
class mafPipe;

#ifdef MAF_EXPORTS
#include "mafDllMacros.h"
EXPORT_STL_VECTOR(MAF_EXPORT,std::string);
#endif

/** Object factory for Pipes.
  To make a new VME available in the MAF it must be plugged inside a factory, in particular
  this factory must be of type mafPipeFactory to be able to retrieve the list of pipes plugged
  in the factory. Also when using mafPlugPipe<pipe_type> the pipe icon is plugged inside the
  the MAF picture factory. */
class MAF_EXPORT mafPipeFactory : public mafObjectFactory
{
public: 
  mafTypeMacro(mafPipeFactory,mafObjectFactory);
  virtual const char* GetMAFSourceVersion() const;
  virtual const char* GetDescription() const;

  /* Initialize the factory creating and registering a new instance */
  static int Initialize();
  /** return the instance pointer of the factory. return NULL if not initialized yet */
  static mafPipeFactory *GetInstance();// {if (!m_Instance) Initialize(); return m_Instance;}

  /** create an instance of the pipe give its type name */
  static mafPipe *CreatePipeInstance(const char *type_name);
   
   /**
    This function can be used by Application code to register new Objects's to the mflCoreFactory */
  void RegisterNewPipe(const char* pipe_name, const char* description, mafCreateObjectFunction createFunction);

  /** return list of names for pipes plugged into this factory */
  static std::vector<std::string> &GetPipeNames();// {return m_PipeNames;}

protected:
  mafPipeFactory();
  ~mafPipeFactory() { }

  // static mafPipeFactory *m_Instance;
  static bool m_Initialized;
  // static std::vector<std::string> m_PipeNames; 
  
private:
  mafPipeFactory(const mafPipeFactory&);  // Not implemented.
  void operator=(const mafPipeFactory&);  // Not implemented.
};

/** Plug  a pipe in the main MAF Pipe factory.*/
template <class T>
class mafPlugPipe
{
  public:
  mafPlugPipe(const char *description);
  
};

//------------------------------------------------------------------------------
/** Plug a new Pipe class into the Pipe factory.*/
template <class T>
mafPlugPipe<T>::mafPlugPipe(const char *description)
//------------------------------------------------------------------------------
{ 
  mafPipeFactory *factory=mafPipeFactory::GetInstance();
  if (factory)
  {
    factory->RegisterNewPipe(T::GetStaticTypeName(), description, T::NewObject);
  }
}

#endif
