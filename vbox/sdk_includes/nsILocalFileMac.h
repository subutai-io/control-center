/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM /home/vbox/tinderbox/sdk/src/libs/xpcom18a4/xpcom/io/nsILocalFileMac.idl
 */

#ifndef __gen_nsILocalFileMac_h__
#define __gen_nsILocalFileMac_h__


#ifndef __gen_nsILocalFile_h__
#include "nsILocalFile.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
#include <Files.h>
#include <CFURL.h>

/* starting interface:    nsILocalFileMac */
#define NS_ILOCALFILEMAC_IID_STR "748f3ffe-27d9-4402-9de9-494badbeebf4"

#define NS_ILOCALFILEMAC_IID \
  {0x748f3ffe, 0x27d9, 0x4402, \
    { 0x9d, 0xe9, 0x49, 0x4b, 0xad, 0xbe, 0xeb, 0xf4 }}

class NS_NO_VTABLE nsILocalFileMac : public nsILocalFile {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ILOCALFILEMAC_IID)

  /**
    * initWithCFURL
    *
    * Init this object with a CFURLRef
    *
    * NOTE: Supported only for XP_MACOSX or TARGET_CARBON
    * NOTE: If the path of the CFURL is /a/b/c, at least a/b must exist beforehand.
    *
    * @param   aCFURL         the CoreFoundation URL
    *
    */
  /* [noscript] void initWithCFURL (in CFURLRef aCFURL); */
  NS_IMETHOD InitWithCFURL(CFURLRef aCFURL) = 0;

  /**
    * initWithFSRef
    *
    * Init this object with an FSRef
    *
    * NOTE: Supported only for XP_MACOSX or TARGET_CARBON
    *
    * @param   aFSRef         the native file spec
    *
    */
  /* [noscript] void initWithFSRef ([const] in FSRefPtr aFSRef); */
  NS_IMETHOD InitWithFSRef(const FSRef * aFSRef) = 0;

  /**
    * initWithFSSpec
    *
    * Init this object with an FSSpec
    * Legacy method - leaving in place for now
    *
    * @param   aFileSpec       the native file spec
    *
    */
  /* [noscript] void initWithFSSpec ([const] in FSSpecPtr aFileSpec); */
  NS_IMETHOD InitWithFSSpec(const FSSpec * aFileSpec) = 0;

  /**
    * initToAppWithCreatorCode
    *
    * Init this object to point to an application having the given
    * creator code. If this app is missing, this will fail. It will first
    * look for running application with the given creator.
    *
    * @param   aAppCreator     the signature of the app
    *
    */
  /* [noscript] void initToAppWithCreatorCode (in OSType aAppCreator); */
  NS_IMETHOD InitToAppWithCreatorCode(OSType aAppCreator) = 0;

  /**
    * getCFURL
    *
    * Returns the CFURLRef of the file object. The caller is
    * responsible for calling CFRelease() on it.
    *
    * NOTE: Observes the state of the followLinks attribute.
    * If the file object is an alias and followLinks is TRUE, returns
    * the target of the alias. If followLinks is FALSE, returns
    * the unresolved alias file.
    *
    * NOTE: Supported only for XP_MACOSX or TARGET_CARBON
    *
    * @return
    *
    */
  /* [noscript] CFURLRef getCFURL (); */
  NS_IMETHOD GetCFURL(CFURLRef *_retval) = 0;

  /**
    * getFSRef
    *
    * Returns the FSRef of the file object.
    *
    * NOTE: Observes the state of the followLinks attribute.
    * If the file object is an alias and followLinks is TRUE, returns
    * the target of the alias. If followLinks is FALSE, returns
    * the unresolved alias file.
    *
    * NOTE: Supported only for XP_MACOSX or TARGET_CARBON
    *
    * @return
    *
    */
  /* [noscript] FSRef getFSRef (); */
  NS_IMETHOD GetFSRef(FSRef *_retval) = 0;

  /**
    * getFSSpec
    *
    * Returns the FSSpec of the file object.
    *
    * NOTE: Observes the state of the followLinks attribute.
    * If the file object is an alias and followLinks is TRUE, returns
    * the target of the alias. If followLinks is FALSE, returns
    * the unresolved alias file.
    *
    * @return
    *
    */
  /* [noscript] FSSpec getFSSpec (); */
  NS_IMETHOD GetFSSpec(FSSpec *_retval) = 0;

  /**
    * fileSizeWithResFork
    *
    * Returns the combined size of both the data fork and the resource
    * fork (if present) rather than just the size of the data fork
    * as returned by GetFileSize()
    *
    */
  /* readonly attribute PRInt64 fileSizeWithResFork; */
  NS_IMETHOD GetFileSizeWithResFork(PRInt64 *aFileSizeWithResFork) = 0;

  /**
    * Use with SetFileType() to specify the signature of current process
    */
  enum { CURRENT_PROCESS_CREATOR = 134217728U };

  /**
    * fileType, creator
    *
    * File type and creator attributes
    *
    */
  /* [noscript] attribute OSType fileType; */
  NS_IMETHOD GetFileType(OSType *aFileType) = 0;
  NS_IMETHOD SetFileType(OSType aFileType) = 0;

  /* [noscript] attribute OSType fileCreator; */
  NS_IMETHOD GetFileCreator(OSType *aFileCreator) = 0;
  NS_IMETHOD SetFileCreator(OSType aFileCreator) = 0;

  /**
    * setFileTypeAndCreatorFromMIMEType
    *
    * Sets the file type and creator code from a MIME type.
    * Internet Config is used to determine the mapping.
    *
    * @param   aMIMEType
    *
    */
  /* void setFileTypeAndCreatorFromMIMEType (in string aMIMEType); */
  NS_IMETHOD SetFileTypeAndCreatorFromMIMEType(const char *aMIMEType) = 0;

  /**
    * setFileTypeAndCreatorFromExtension
    *
    * Sets the file type and creator code from a file extension
    * Internet Config is used to determine the mapping.
    *
    * @param   aExtension
    *
    */
  /* void setFileTypeAndCreatorFromExtension (in string aExtension); */
  NS_IMETHOD SetFileTypeAndCreatorFromExtension(const char *aExtension) = 0;

  /**
    * launchWithDoc
    *
    * Launch the application that this file points to with a document.
    *
    * @param   aDocToLoad          Must not be NULL. If no document, use nsILocalFile::launch
    * @param   aLaunchInBackground TRUE if the application should not come to the front.
    *
    */
  /* void launchWithDoc (in nsILocalFile aDocToLoad, in boolean aLaunchInBackground); */
  NS_IMETHOD LaunchWithDoc(nsILocalFile *aDocToLoad, PRBool aLaunchInBackground) = 0;

  /**
    * openDocWithApp
    *
    * Open the document that this file points to with the given application.
    *
    * @param   aAppToOpenWith      The application with  which to open the document.
    *                              If NULL, the creator code of the document is used
    *                              to determine the application.
    * @param   aLaunchInBackground TRUE if the application should not come to the front.
    *
    */
  /* void openDocWithApp (in nsILocalFile aAppToOpenWith, in boolean aLaunchInBackground); */
  NS_IMETHOD OpenDocWithApp(nsILocalFile *aAppToOpenWith, PRBool aLaunchInBackground) = 0;

  /**
    * isPackage
    *
    * returns true if a directory is determined to be a package under Mac OS 9/X
    *
    */
  /* boolean isPackage (); */
  NS_IMETHOD IsPackage(PRBool *_retval) = 0;

  /**
    * bundleDisplayName
    *
    * returns the display name of the application bundle (usually the human
    * readable name of the application)
    */
  /* readonly attribute AString bundleDisplayName; */
  NS_IMETHOD GetBundleDisplayName(nsAString & aBundleDisplayName) = 0;

  /**
    * bundleIdentifier
    *
    * returns the identifier of the bundle
    */
  /* readonly attribute AUTF8String bundleIdentifier; */
  NS_IMETHOD GetBundleIdentifier(nsACString & aBundleIdentifier) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSILOCALFILEMAC \
  NS_IMETHOD InitWithCFURL(CFURLRef aCFURL); \
  NS_IMETHOD InitWithFSRef(const FSRef * aFSRef); \
  NS_IMETHOD InitWithFSSpec(const FSSpec * aFileSpec); \
  NS_IMETHOD InitToAppWithCreatorCode(OSType aAppCreator); \
  NS_IMETHOD GetCFURL(CFURLRef *_retval); \
  NS_IMETHOD GetFSRef(FSRef *_retval); \
  NS_IMETHOD GetFSSpec(FSSpec *_retval); \
  NS_IMETHOD GetFileSizeWithResFork(PRInt64 *aFileSizeWithResFork); \
  NS_IMETHOD GetFileType(OSType *aFileType); \
  NS_IMETHOD SetFileType(OSType aFileType); \
  NS_IMETHOD GetFileCreator(OSType *aFileCreator); \
  NS_IMETHOD SetFileCreator(OSType aFileCreator); \
  NS_IMETHOD SetFileTypeAndCreatorFromMIMEType(const char *aMIMEType); \
  NS_IMETHOD SetFileTypeAndCreatorFromExtension(const char *aExtension); \
  NS_IMETHOD LaunchWithDoc(nsILocalFile *aDocToLoad, PRBool aLaunchInBackground); \
  NS_IMETHOD OpenDocWithApp(nsILocalFile *aAppToOpenWith, PRBool aLaunchInBackground); \
  NS_IMETHOD IsPackage(PRBool *_retval); \
  NS_IMETHOD GetBundleDisplayName(nsAString & aBundleDisplayName); \
  NS_IMETHOD GetBundleIdentifier(nsACString & aBundleIdentifier); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSILOCALFILEMAC(_to) \
  NS_IMETHOD InitWithCFURL(CFURLRef aCFURL) { return _to InitWithCFURL(aCFURL); } \
  NS_IMETHOD InitWithFSRef(const FSRef * aFSRef) { return _to InitWithFSRef(aFSRef); } \
  NS_IMETHOD InitWithFSSpec(const FSSpec * aFileSpec) { return _to InitWithFSSpec(aFileSpec); } \
  NS_IMETHOD InitToAppWithCreatorCode(OSType aAppCreator) { return _to InitToAppWithCreatorCode(aAppCreator); } \
  NS_IMETHOD GetCFURL(CFURLRef *_retval) { return _to GetCFURL(_retval); } \
  NS_IMETHOD GetFSRef(FSRef *_retval) { return _to GetFSRef(_retval); } \
  NS_IMETHOD GetFSSpec(FSSpec *_retval) { return _to GetFSSpec(_retval); } \
  NS_IMETHOD GetFileSizeWithResFork(PRInt64 *aFileSizeWithResFork) { return _to GetFileSizeWithResFork(aFileSizeWithResFork); } \
  NS_IMETHOD GetFileType(OSType *aFileType) { return _to GetFileType(aFileType); } \
  NS_IMETHOD SetFileType(OSType aFileType) { return _to SetFileType(aFileType); } \
  NS_IMETHOD GetFileCreator(OSType *aFileCreator) { return _to GetFileCreator(aFileCreator); } \
  NS_IMETHOD SetFileCreator(OSType aFileCreator) { return _to SetFileCreator(aFileCreator); } \
  NS_IMETHOD SetFileTypeAndCreatorFromMIMEType(const char *aMIMEType) { return _to SetFileTypeAndCreatorFromMIMEType(aMIMEType); } \
  NS_IMETHOD SetFileTypeAndCreatorFromExtension(const char *aExtension) { return _to SetFileTypeAndCreatorFromExtension(aExtension); } \
  NS_IMETHOD LaunchWithDoc(nsILocalFile *aDocToLoad, PRBool aLaunchInBackground) { return _to LaunchWithDoc(aDocToLoad, aLaunchInBackground); } \
  NS_IMETHOD OpenDocWithApp(nsILocalFile *aAppToOpenWith, PRBool aLaunchInBackground) { return _to OpenDocWithApp(aAppToOpenWith, aLaunchInBackground); } \
  NS_IMETHOD IsPackage(PRBool *_retval) { return _to IsPackage(_retval); } \
  NS_IMETHOD GetBundleDisplayName(nsAString & aBundleDisplayName) { return _to GetBundleDisplayName(aBundleDisplayName); } \
  NS_IMETHOD GetBundleIdentifier(nsACString & aBundleIdentifier) { return _to GetBundleIdentifier(aBundleIdentifier); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSILOCALFILEMAC(_to) \
  NS_IMETHOD InitWithCFURL(CFURLRef aCFURL) { return !_to ? NS_ERROR_NULL_POINTER : _to->InitWithCFURL(aCFURL); } \
  NS_IMETHOD InitWithFSRef(const FSRef * aFSRef) { return !_to ? NS_ERROR_NULL_POINTER : _to->InitWithFSRef(aFSRef); } \
  NS_IMETHOD InitWithFSSpec(const FSSpec * aFileSpec) { return !_to ? NS_ERROR_NULL_POINTER : _to->InitWithFSSpec(aFileSpec); } \
  NS_IMETHOD InitToAppWithCreatorCode(OSType aAppCreator) { return !_to ? NS_ERROR_NULL_POINTER : _to->InitToAppWithCreatorCode(aAppCreator); } \
  NS_IMETHOD GetCFURL(CFURLRef *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetCFURL(_retval); } \
  NS_IMETHOD GetFSRef(FSRef *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetFSRef(_retval); } \
  NS_IMETHOD GetFSSpec(FSSpec *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetFSSpec(_retval); } \
  NS_IMETHOD GetFileSizeWithResFork(PRInt64 *aFileSizeWithResFork) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetFileSizeWithResFork(aFileSizeWithResFork); } \
  NS_IMETHOD GetFileType(OSType *aFileType) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetFileType(aFileType); } \
  NS_IMETHOD SetFileType(OSType aFileType) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetFileType(aFileType); } \
  NS_IMETHOD GetFileCreator(OSType *aFileCreator) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetFileCreator(aFileCreator); } \
  NS_IMETHOD SetFileCreator(OSType aFileCreator) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetFileCreator(aFileCreator); } \
  NS_IMETHOD SetFileTypeAndCreatorFromMIMEType(const char *aMIMEType) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetFileTypeAndCreatorFromMIMEType(aMIMEType); } \
  NS_IMETHOD SetFileTypeAndCreatorFromExtension(const char *aExtension) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetFileTypeAndCreatorFromExtension(aExtension); } \
  NS_IMETHOD LaunchWithDoc(nsILocalFile *aDocToLoad, PRBool aLaunchInBackground) { return !_to ? NS_ERROR_NULL_POINTER : _to->LaunchWithDoc(aDocToLoad, aLaunchInBackground); } \
  NS_IMETHOD OpenDocWithApp(nsILocalFile *aAppToOpenWith, PRBool aLaunchInBackground) { return !_to ? NS_ERROR_NULL_POINTER : _to->OpenDocWithApp(aAppToOpenWith, aLaunchInBackground); } \
  NS_IMETHOD IsPackage(PRBool *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->IsPackage(_retval); } \
  NS_IMETHOD GetBundleDisplayName(nsAString & aBundleDisplayName) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetBundleDisplayName(aBundleDisplayName); } \
  NS_IMETHOD GetBundleIdentifier(nsACString & aBundleIdentifier) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetBundleIdentifier(aBundleIdentifier); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsLocalFileMac : public nsILocalFileMac
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSILOCALFILEMAC

  nsLocalFileMac();

private:
  ~nsLocalFileMac();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsLocalFileMac, nsILocalFileMac)

nsLocalFileMac::nsLocalFileMac()
{
  /* member initializers and constructor code */
}

nsLocalFileMac::~nsLocalFileMac()
{
  /* destructor code */
}

/* [noscript] void initWithCFURL (in CFURLRef aCFURL); */
NS_IMETHODIMP nsLocalFileMac::InitWithCFURL(CFURLRef aCFURL)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] void initWithFSRef ([const] in FSRefPtr aFSRef); */
NS_IMETHODIMP nsLocalFileMac::InitWithFSRef(const FSRef * aFSRef)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] void initWithFSSpec ([const] in FSSpecPtr aFileSpec); */
NS_IMETHODIMP nsLocalFileMac::InitWithFSSpec(const FSSpec * aFileSpec)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] void initToAppWithCreatorCode (in OSType aAppCreator); */
NS_IMETHODIMP nsLocalFileMac::InitToAppWithCreatorCode(OSType aAppCreator)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] CFURLRef getCFURL (); */
NS_IMETHODIMP nsLocalFileMac::GetCFURL(CFURLRef *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] FSRef getFSRef (); */
NS_IMETHODIMP nsLocalFileMac::GetFSRef(FSRef *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] FSSpec getFSSpec (); */
NS_IMETHODIMP nsLocalFileMac::GetFSSpec(FSSpec *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute PRInt64 fileSizeWithResFork; */
NS_IMETHODIMP nsLocalFileMac::GetFileSizeWithResFork(PRInt64 *aFileSizeWithResFork)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] attribute OSType fileType; */
NS_IMETHODIMP nsLocalFileMac::GetFileType(OSType *aFileType)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsLocalFileMac::SetFileType(OSType aFileType)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] attribute OSType fileCreator; */
NS_IMETHODIMP nsLocalFileMac::GetFileCreator(OSType *aFileCreator)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsLocalFileMac::SetFileCreator(OSType aFileCreator)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void setFileTypeAndCreatorFromMIMEType (in string aMIMEType); */
NS_IMETHODIMP nsLocalFileMac::SetFileTypeAndCreatorFromMIMEType(const char *aMIMEType)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void setFileTypeAndCreatorFromExtension (in string aExtension); */
NS_IMETHODIMP nsLocalFileMac::SetFileTypeAndCreatorFromExtension(const char *aExtension)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void launchWithDoc (in nsILocalFile aDocToLoad, in boolean aLaunchInBackground); */
NS_IMETHODIMP nsLocalFileMac::LaunchWithDoc(nsILocalFile *aDocToLoad, PRBool aLaunchInBackground)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void openDocWithApp (in nsILocalFile aAppToOpenWith, in boolean aLaunchInBackground); */
NS_IMETHODIMP nsLocalFileMac::OpenDocWithApp(nsILocalFile *aAppToOpenWith, PRBool aLaunchInBackground)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean isPackage (); */
NS_IMETHODIMP nsLocalFileMac::IsPackage(PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute AString bundleDisplayName; */
NS_IMETHODIMP nsLocalFileMac::GetBundleDisplayName(nsAString & aBundleDisplayName)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute AUTF8String bundleIdentifier; */
NS_IMETHODIMP nsLocalFileMac::GetBundleIdentifier(nsACString & aBundleIdentifier)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif

extern "C"
{
#ifndef XP_MACOSX
NS_EXPORT const char* NS_TruncNodeName(const char *aNode, char *outBuf);
#endif
NS_EXPORT nsresult NS_NewLocalFileWithFSSpec(const FSSpec* inSpec, PRBool followSymlinks, nsILocalFileMac* *result);
// NS_NewLocalFileWithFSRef is available since Mozilla 1.8.1.
NS_EXPORT nsresult NS_NewLocalFileWithFSRef(const FSRef* aFSRef, PRBool aFollowSymlinks, nsILocalFileMac** result);
}

#endif /* __gen_nsILocalFileMac_h__ */
