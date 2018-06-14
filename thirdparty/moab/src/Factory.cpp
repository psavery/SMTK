/**
 * MOAB, a Mesh-Oriented datABase, is a software component for creating,
 * storing and accessing finite element mesh data.
 * 
 * Copyright 2004 Sandia Corporation.  Under the terms of Contract
 * DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government
 * retains certain rights in this software.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 */

#ifdef _WIN32
#ifdef _DEBUG
// turn off warnings that say they debugging identifier has been truncated
// this warning comes up when using some STL containers
#pragma warning(disable : 4786)
#endif
#endif

#include "moab/Core.hpp"

#ifdef XPCOM_MB

#include "nsIGenericFactory.h"

// define constructor function for Core
NS_GENERIC_FACTORY_CONSTRUCTOR(moab::Core)

// support for nsIClassInfo
NS_DECL_CLASSINFO(moab::Core)

MB_EXPORT const char* MoabVersion();
MB_EXPORT void GetInterface(MBuuid& interface_requested, UnknownInterface** iface);
MB_EXPORT void DeInitialize();
MB_EXPORT void ReleaseInterface(UnknownInterface* iface);

static const nsModuleComponentInfo components[] =
{
  { "MOAB Interface", CORE_CID, CORE_CONTRACTID, CoreConstructor,
    NULL /* NULL if you dont need one */,
    NULL /* NULL if you dont need one */,
    NULL /* no factory destructor */,
    NS_CI_INTERFACE_GETTER_NAME(moab::Core),
    NULL /* no language helper */,
    &NS_CLASSINFO_NAME(moab::Core),
    0
  }
};

// implement NSGetModule()
NS_IMPL_NSGETMODULE(moab::Core, components);

#endif

#ifndef _WIN32
  #define MB_EXPORT extern "C"
#else
  #define MB_EXPORT extern "C" __declspec(dllexport)
#endif

#include <list>

namespace moab {

class ComponentFactory : public UnknownInterface
{
public:
   ComponentFactory(){}
   virtual ~ComponentFactory(){}
   // returns the interface requested from an object
   virtual int QueryInterface( const MBuuid&,
         UnknownInterface** );
   // keep track of the objects this component factory creates
   static std::list<UnknownInterface*> objects_in_use;
};

// the list that keeps track of all the interfaces generated by this server
std::list<UnknownInterface*> ComponentFactory::objects_in_use;


// this QueryInterface function is supposed to create an instance of the object
// that contains the interface requested
//
// note: the object is not the same as the interface, therefore
// we ask the object for the interface that was requested
//

int ComponentFactory::QueryInterface( const MBuuid& uuid, UnknownInterface** iface )
{
   // this is an unknown interface that was requested
   // if wanted, we could provide a default interface
   // if IDD_MBUnknown is specified
   if(uuid == IDD_MBUnknown)
      return 0;
   // IDD_MBVerde interface was requested
   // create an Verde object and have it return the interface
   // requested
   else if(uuid == IDD_MBCore)
   {
      Core* mdb = new Core;
      // if the object does not contain the interface requested, delete the object
      if(!mdb->QueryInterface(uuid, iface))
      {
         delete mdb;
         return 0;
      }
      return 1;
   }
   else
      return 0;

}

// returns the interface version
MB_EXPORT const char* MoabVersion()
{
   return MB_INTERFACE_VERSION;
}


// Initialize function is accessed by the MBClient when asking for interfaces
MB_EXPORT void GetInterface(MBuuid& interface_requested, UnknownInterface** iface)
{
   // create an instance of our component factory
   ComponentFactory server;
   // ask the component factory to give us the interface we want
   server.QueryInterface(interface_requested, iface);
   // if the interface existed, put it on our list
   if(iface && *iface)
      ComponentFactory::objects_in_use.push_front(*iface);
}


// DeInitialize function is accessed by the MBClient when disconnecting from this library
// this will clean everything up prior to a disconnection
// from this library
MB_EXPORT void DeInitialize()
{
   // delete all instances of objects
   while(ComponentFactory::objects_in_use.size())
   {
      UnknownInterface* iface =
         ComponentFactory::objects_in_use.front();
      ComponentFactory::objects_in_use.pop_front();
      if(iface)
         delete iface;
   }
}

// ReleaseInterface function is accessed by the MBClient when deleting an interface

// ReleaseInterface will delete this interface
MB_EXPORT void ReleaseInterface(UnknownInterface* iface)
{
   if(!iface)
      return;
   // remove this object from our list and delete it
   ComponentFactory::objects_in_use.remove(iface);
   delete iface;
}
  
} // namespace moab

