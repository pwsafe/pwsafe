/// \file PwsBackend.cpp
/// \brief The "global" backend object that the frontend controls
//-----------------------------------------------------------------------------

#include "PwsBackend.h"

//-----------------------------------------------------------------------------
PwsBackend::PwsBackend()
{
   srand((unsigned)time(NULL));
}


PwsBackend::~PwsBackend()
{
   //We no longer need the global bf_P and bf_S variables, so trash them
   trashMemory((unsigned char*)tempbf_P, 18*4);
   trashMemory((unsigned char*)tempbf_S, 256*4);
   trashMemory((unsigned char*)bf_P, 18*4);
   trashMemory((unsigned char*)bf_S, 256*4);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
