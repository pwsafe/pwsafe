// PwsBackend.h
// Password Safe - backend
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
class PwsBackend
{
public:
   PwsBackend();
   ~PwsBackend();
   
   CMyString m_passkey; // the main one, in memory?!? yikes {jpr}

   unsigned char m_randstuff[StuffSize];
   unsigned char m_randhash[SaltSize];

};

//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
