#ifndef MRE_WRAPPER_H
#define MRE_WRAPPER_H

//Manual reset event wrapper
template <class T> class MRE_Wrapper {
private:
  MRE_Wrapper(void);
  MRE_Wrapper(const MRE_Wrapper&);
  void operator=(const MRE_Wrapper&);

public:
  static int MRE_Init(T* lpMre){return T::MRE_Init(lpMre);}

  static int MRE_Wait(T* lpMre, int timeInMs){return T::MRE_Wait(lpMre, timeInMs);}

  static int MRE_Set(T* lpMre){return T::MRE_Set(lpMre);}

  static int MRE_Reset(T* lpMre){return T::MRE_Reset(lpMre);}

  static int MRE_Destroy(T* lpMre){return T::MRE_Destroy(lpMre);}
};

#endif // MRE_WRAPPER_H
