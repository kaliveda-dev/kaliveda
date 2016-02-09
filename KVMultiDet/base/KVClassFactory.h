/*
$Id: KVClassFactory.h,v 1.13 2009/01/21 08:04:20 franklan Exp $
$Revision: 1.13 $
$Date: 2009/01/21 08:04:20 $
*/

#ifndef __KVCLASSFACTORY_H
#define __KVCLASSFACTORY_H

#include "Riostream.h"
#include "TDatime.h"
#include "TClass.h"
#include "KVString.h"
#include "KVNameValueList.h"
#include "KVList.h"
#include "KVBase.h"

class KVClassMember : public KVBase {

protected:
   KVString fComment;//informative comment for member variable, like this one :)

public:
   KVClassMember() {};
   KVClassMember(const Char_t*, const Char_t*, const Char_t*, const Char_t* = "protected");
   KVClassMember(const KVClassMember&);
   virtual ~KVClassMember() {};
   void Copy(TObject& obj) const;

   void Print(Option_t* = "") const;

   virtual void WriteDeclaration(KVString&);

   // set access type : public, protected or private
   void SetAccess(const Char_t* acc = "public")
   {
      SetLabel(acc);
   }

   // get access type : public, protected or private
   const Char_t* GetAccess() const
   {
      return GetLabel();
   }

   // set comment for variable
   void SetComment(const Char_t* c)
   {
      fComment = c;
   }

   // get access type : public, protected or private
   const Char_t* GetComment() const
   {
      return fComment;
   }

   ClassDef(KVClassMember, 1) //KVClassFactory helper class - description of class member variable
};

//_____________________________________________________________________________

class KVClassMethod : public KVClassMember {

   KVNameValueList fFields;//fields of method declaration
   Bool_t fVirtual;//kTRUE if method is 'virtual'
   Bool_t fConst;//kTRUE if method is 'const'
   Bool_t fConstructor;//kTRUE if method is a constructor
   Bool_t fCopyCtor;//kTRUE if method is the copy constructor
   int fNargs;//counts arguments

public:

   Int_t GetNargs() const
   {
      return fNargs;
   };
   void SetNargs(Int_t n)
   {
      fNargs = n;
   };

   KVClassMethod()
   {
      fNargs = 0;
      fVirtual = fConst = fConstructor = fCopyCtor = kFALSE;
   };
   KVClassMethod(const KVClassMethod&);
   virtual ~KVClassMethod() {};
   void Copy(TObject& obj) const;

   void SetReturnType(const Char_t* type)
   {
      KVString s(type);
      fFields.SetValue("ReturnType", s);
   };
   void SetClassName(const Char_t* name)
   {
      KVString s(name);
      fFields.SetValue("ClassName", s);
   };
   void SetBaseClass(const Char_t* name)
   {
      KVString s(name);
      fFields.SetValue("BaseClass", s);
   };
   void AddArgument(const Char_t* type, const Char_t* argname = "", const Char_t* defaultvalue = "")
   {
      KVString _type(type);
      fFields.SetValue(Form("Arg_%d", ++fNargs), _type);
      if (strcmp(defaultvalue, "")) {
         KVString _s(defaultvalue);
         fFields.SetValue(Form("Arg_%d_default", fNargs), _s);
      }
      if (strcmp(argname, "")) {
         KVString _s(argname);
         fFields.SetValue(Form("Arg_%d_name", fNargs), _s);
      }
   };

   void SetMethodBody(const KVString& body)
   {
      fFields.SetValue("Body", body);
   };

   const Char_t* GetReturnType()
   {
      if (fFields.HasParameter("ReturnType"))
         return fFields.GetStringValue("ReturnType");
      return "";
   };
   const Char_t* GetClassName()
   {
      return fFields.GetStringValue("ClassName");
   };

   void SetConst(Bool_t c = kTRUE)
   {
      fConst = c;
   };
   void SetVirtual(Bool_t c = kTRUE)
   {
      fVirtual = c;
   };
   void SetConstructor(Bool_t c = kTRUE)
   {
      fConstructor = c;
   };
   void SetCopyCtor(Bool_t c = kTRUE)
   {
      fCopyCtor = c;
   }

   Bool_t IsConst() const
   {
      return fConst;
   };
   Bool_t IsVirtual() const
   {
      return fVirtual;
   };
   Bool_t IsConstructor() const
   {
      return fConstructor;
   };
   Bool_t IsCopyCtor() const
   {
      return fCopyCtor;
   }

   void Print(Option_t* = "") const;

   void WriteDeclaration(KVString&);
   void WriteImplementation(KVString& decl);

   ClassDef(KVClassMethod, 2) //KVClassFactory helper class - description of class method
};

//_____________________________________________________________________________

class KVClassFactory : public TObject {

private:

   KVString fClassName;   //name of class to generate
   KVString fClassDesc;   //class description
   KVString fBaseClassName;   //name of base class
   TClass* fBaseClass; //! description of base class
   Bool_t fHasBaseClass; //kTRUE if class derived from another
   Bool_t fBaseClassTObject; //kTRUE if class derived from TObject
   KVString fAuthor;      //user who called ClassFactory to generate class
   TDatime fNow;         //for dating files
   Bool_t fWithTemplate;  //true if class has a template
   KVString fTemplateBase; //template base name passed to SetTemplate method
   KVString fTemplateClassName;   //name of template dummy class
   KVString fTemplateH;   //full path to template .h
   KVString fTemplateCPP; //full path to template .cpp
   KVList fMembers;//list of member variables for class
   KVList fMethods;//list of methods added to class
   KVList fHeadInc;//list of 'includes' to be added to header file
   KVList fImpInc;//list of 'includes' to be added to implementation file

protected:

   void WritePreProc(std::ofstream&);
   void WriteWhoWhen(std::ofstream&);
   void SetWhoWhen();
   void WriteClassDec(std::ofstream&);
   void WriteClassHeader();
   void WriteClassImp();
   void WriteClassWithTemplateHeader();
   void WriteClassWithTemplateImp();
   Bool_t CheckTemplateFiles(const Char_t* base_class,
                             const Char_t* templateFile);


   Ssiz_t FindNextUncommentedLine(TString&, Ssiz_t beg = 0);
   void AddTObjectCopyMethod();
   void AddCopyConstructor(Bool_t withTObjectCopy = kFALSE);

public:

   KVClassFactory();
   KVClassFactory(const Char_t* classname,
                  const Char_t* classdesc,
                  const Char_t* base_class =
                     "", Bool_t withTemplate =
                     kFALSE, const Char_t* templateFile = "");
   KVClassFactory(const KVClassFactory&);
   virtual ~KVClassFactory() {};
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   void Copy(TObject& obj) const;
#else
   void Copy(TObject& obj);
#endif

   static void MakeClass(const Char_t* classname,
                         const Char_t* classdesc,
                         const Char_t* base_class =
                            "", Bool_t withTemplate =
                            kFALSE, const Char_t* templateFile = "");
   void GenerateCode();

   KVClassMember* AddMember(const Char_t* name, const Char_t* type, const Char_t* comment, const Char_t* access = "protected");
   KVClassMethod* AddMethod(const Char_t* name, const Char_t* return_type, const Char_t* access = "public",
                            Bool_t isVirtual = kFALSE, Bool_t isConst = kFALSE);
   KVClassMethod* AddConstructor(const Char_t* argument_type,
                                 const Char_t* argument_name = "", const Char_t* default_value = "", const Char_t* access = "public");
   void AddAllBaseConstructors();
   void AddMethod(const KVClassMethod& kvcm);
   void AddMethodArgument(const Char_t* method_name, const Char_t* argument_type,
                          const Char_t* argument_name = "", const Char_t* default_value = "");
   void AddMethodBody(const Char_t* method_name, KVString& body);

   void AddHeaderIncludeFile(const Char_t* filename);
   void AddImplIncludeFile(const Char_t* filename);

   const KVList* GetListOfMethods()
   {
      return &fMethods;
   };

   const Char_t* GetClassName() const
   {
      return fClassName.Data();
   };
   void SetClassName(const Char_t* n)
   {
      fClassName = n;
   };
   const Char_t* GetHeaderFileName() const
   {
      return Form("%s.h", fClassName.Data());
   };
   const Char_t* GetImpFileName() const
   {
      return Form("%s.cpp", fClassName.Data());
   };
   void SetClassDesc(const Char_t* d)
   {
      fClassDesc = d;
   };
   const Char_t* GetClassDesc() const
   {
      return fClassDesc.Data();
   };
   void SetBaseClass(const Char_t* b)
   {
      fBaseClassName = b;
      fHasBaseClass = (fBaseClassName != "");
      fBaseClassTObject = kFALSE;
      if (fHasBaseClass) {
         fBaseClass = TClass::GetClass(fBaseClassName);
         fBaseClassTObject = (fBaseClass && fBaseClass->InheritsFrom("TObject"));
      }
   };
   const Char_t* GetBaseClass() const
   {
      return fBaseClassName.Data();
   };
   Bool_t WithMultipleBaseClasses() const
   {
      return fBaseClassName.Contains(",");
   };
   Bool_t WithTemplate() const
   {
      return fWithTemplate;
   };
   void SetTemplate(Bool_t temp, const Char_t* temp_file);
   const Char_t* GetTemplateBase() const
   {
      return fTemplateBase;
   };

   void Print(Option_t* opt = "") const;

   void AddGetSetMethods(const KVNameValueList&);

   ClassDef(KVClassFactory, 4) //Factory for generating KaliVeda skeleton classes
};

#endif
