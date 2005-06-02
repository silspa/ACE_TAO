/**
 * @file NAM_Map.h
 * @author Will Otte <wotte@dre.vanderbilt.edu>
 *
 * Map of NodeApplicationManagers
 *
 * $Id$
 */

#ifndef CIAO_NAM_MAP_H
#define CIAO_NAM_MAP_H

#include /**/ "ace/pre.h"

#include "ace/Null_Mutex.h"
#include "ace/Hash_Map_Manager.h"
#include "ciao/DeploymentC.h"
#include "ace/SString.h"
#include "tao/PortableServer/PortableServer.h"
#if !defined (ACE_LACKS_PRAGMA_ONCE)
# pragma once
#endif /* ACE_LACKS_PRAGMA_ONCE */

namespace CIAO
{
  /**
   * @class NAM_Map
   * @brief Implementation of a map of NodeApplicationManagers.
   *
   * This table is used by the Node_Daemon to keep track of 
   * NAMs started for multiple assemblies. 
   */
  class NAM_Map
  {
  public:
    /// Constructor
    NAM_Map (void);
    
    /// Determine if there is a NAM associated with a UUID
    bool is_available (const ACE_CString &str);
    
    /// Insert a NAM OID into the collection
    bool insert_nam (const ACE_CString &str,
                     const PortableServer::ObjectId &oid);
                
    /// Get a specified NAM.  
    ::PortableServer::ObjectId
    get_nam (const ACE_CString &str);
    
    /// Remove a nam from the map, given its oid.
    bool remove_nam (const PortableServer::ObjectId &oid);
    
  private:
    typedef
    ACE_Hash_Map_Manager_Ex <ACE_CString,
                             ::PortableServer::ObjectId_var,
                             ACE_Hash<ACE_CString>,
                             ACE_Equal_To<ACE_CString>,
                             ACE_Null_Mutex> MAP;
    
    typedef MAP::iterator Iterator;
    
    MAP map_;
  };
}

    

#endif /*CIAO_NAM_MAP_H*/
