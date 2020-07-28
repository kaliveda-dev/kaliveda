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


/**
  \class KVClassFactory
  \brief  Factory class for generating skeleton files for new classes.
  \ingroup Core

It can generate basic '.h' and '.cpp' files for
      - base classes (base_class="")
      - inherited classes (base_class!="")
      - classes based on template files

There are two ways to use KVClassFactory in order to generate code:

1. Using the static (stand-alone) method
~~~~~{.cpp}
      KVClassFactory::MakeClass(classname, classdesc, base_class, withTemplate, templateFile);
~~~~~
2. Create and configure a KVClassFactory object and then call the GenerateCode() method:
~~~~~{.cpp}
    KVClassFactory fact(classname, classdesc, base_class, withTemplate, templateFile);
    fact.GenerateCode();
~~~~~
The second method is more flexible and allows to add methods to classes before
code generation, even if the class is created from a predefined template.

For example, let us suppose that we want to add the following method to our class:
~~~~~~{.cpp}
   -//-> declaration:
      virtual const Char_t* GetName(Option_t* = "") const;

   -//-> implementation:
      const Char_t* MyNewClass::GetName(Option_t* opt) const
      {
            //A new method
            if( strcmp(opt,"") ) cout << "Option : " << opt << endl;
            else cout << fName.Data() << endl;
            return fName.Data();
      }
~~~~~~

This can be done as follows:

~~~~~~{.cpp}
    KVClassFactory fact("MyNewClass", "My latest creation");
    fact.AddMethod("GetName", "const Char_t*", kTRUE, kTRUE);
    fact.AddMethodArgument("GetName", "Option_t*", "opt", "\"\"");
    KVString body;
    body += "      //A new method\n";
    body += "      if( strcmp(opt,\"\") ) std::cout << \"Option : \" << opt << std::endl;\n";
    body += "      else std::cout << fName.Data() << std::endl;\n";
    body += "      return fName.Data();";
    fact.AddMethodBody("GetName", body);
~~~~~~

The addition of a new method may mean that it is necessary to add an '#include'
directive to either the header or the implementation file of the new class. For example,
in this case, the use of 'cout', 'endl' etc. may require to add an '#include <iostream>'
to the '.cpp' file of the new class. This can be done as follows:

~~~~~~{.cpp}
    fact.AddImplIncludeFile("iostream");
    fact.GenerateCode();
~~~~~~

For another example of this kind of approach, see the method KVParticleCondition::Optimize.
*/
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
   Bool_t fInlineAllMethods;//kTRUE if all (non-ctor) method implementations written in header
   Bool_t fInlineAllCtors;//kTRUE if all ctor implementations written in header
   KVString fClassPath;//directory in which to write source files, if not working directory
   Bool_t fInheritAllCtors;//kTRUE if all ctor from base class should be inherited

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
   void AddCopyConstructor();

   void GenerateGettersAndSetters();
   void AddAssignmentOperator();
public:

   /**
     \class KVClassMember
     \brief Member variable in a class generated with KVClassFactory
    */
   class KVClassMember : public KVBase {

   protected:
      KVString fComment;//informative comment for member variable, like this one :)
      KVString fRealName;//the name of the member without the leading 'f'

   public:
      KVClassMember()
      {
         SetAccess();
      }
      KVClassMember(const Char_t*, const Char_t*, const Char_t*, const Char_t* = "protected");
      KVClassMember(const KVClassMember&);
      ROOT_COPY_ASSIGN_OP(KVClassMember)
      virtual ~KVClassMember() {}
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
      Bool_t IsPublic() const
      {
         // return kTRUE for public member/method
         return !strcmp(GetAccess(), "public");
      }
      Bool_t IsProtected() const
      {
         // return kTRUE for protected member/method
         return !strcmp(GetAccess(), "protected");
      }
      Bool_t IsPrivate() const
      {
         // return kTRUE for private member/method
         return !strcmp(GetAccess(), "private");
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

      // get short name without leading 'f'
      const Char_t* GetRealName() const
      {
         return fRealName;
      }

      ClassDef(KVClassMember, 1) //KVClassFactory helper class - description of class member variable
   };

   //_____________________________________________________________________________

   /**
     \class KVClassMethod
     \brief Method in a class generated with KVClassFactory
    */

   class KVClassMethod : public KVClassMember {

   protected:
      virtual void write_method_body(KVString& decl);
      KVNameValueList fFields;//fields of method declaration
      Bool_t fVirtual;//kTRUE if method is 'virtual'
      Bool_t fConst;//kTRUE if method is 'const'
      int fNargs;//counts arguments
      Bool_t fInline;//kTRUE if method body is to be written in header file

   public:

      Int_t GetNargs() const
      {
         return fNargs;
      }
      void SetNargs(Int_t n)
      {
         fNargs = n;
      }

      KVClassMethod(Bool_t Virtual = kFALSE, Bool_t Const = kFALSE, Bool_t Inline = kFALSE)
         : KVClassMember(), fVirtual(Virtual), fConst(Const), fNargs(0), fInline(Inline)
      {
      }
      KVClassMethod(const KVClassMethod&);
      virtual ~KVClassMethod() {}
      void Copy(TObject& obj) const;

      void SetReturnType(const Char_t* type)
      {
         KVString s(type);
         fFields.SetValue("ReturnType", s);
      }
      void SetClassName(const Char_t* name)
      {
         KVString s(name);
         fFields.SetValue("ClassName", s);
      }
      void SetBaseClass(const Char_t* name)
      {
         KVString s(name);
         fFields.SetValue("BaseClass", s);
      }
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
      }
      void AddArgument(Int_t i, const Char_t* type, const Char_t* argname = "", const Char_t* defaultvalue = "");
      void SetMethodBody(const KVString& body)
      {
         fFields.SetValue("Body", body);
      }

      const Char_t* GetReturnType()
      {
         if (fFields.HasParameter("ReturnType"))
            return fFields.GetStringValue("ReturnType");
         return "";
      }
      const Char_t* GetClassName()
      {
         return fFields.GetStringValue("ClassName");
      }

      void SetConst(Bool_t c = kTRUE)
      {
         fConst = c;
      }
      void SetVirtual(Bool_t c = kTRUE)
      {
         fVirtual = c;
      }

      Bool_t IsConst() const
      {
         return fConst;
      }
      Bool_t IsVirtual() const
      {
         return fVirtual;
      }
      virtual Bool_t IsConstructor() const
      {
         return kFALSE;
      }

      void Print(Option_t* = "") const;

      void WriteDeclaration(KVString&);
      void WriteImplementation(KVString& decl);

      void SetInline(Bool_t yes = kTRUE)
      {
         fInline = yes;
      }
      Bool_t IsInline() const
      {
         return fInline;
      }
      virtual Bool_t IsDestructor() const
      {
         return kFALSE;
      }
      Bool_t IsNormalMethod() const
      {
         // return kTRUE if method is neither constructor nor destructor
         return (!IsConstructor() && !IsDestructor());
      }

      ClassDef(KVClassMethod, 2) //KVClassFactory helper class - description of class method
   };

   /**
     \class KVClassConstructor
     \brief Constructor for a class generated with KVClassFactory
    */

   class KVClassConstructor : public KVClassMethod {
      Bool_t fCopyCtor;//kTRUE if method is the copy constructor
      virtual void write_method_body(KVString& decl);
      KVClassFactory* fParentClass;

   public:
      KVClassConstructor(KVClassFactory* ParentClass)
         : KVClassMethod(), fCopyCtor(kFALSE), fParentClass(ParentClass) {};
      virtual ~KVClassConstructor() {};

      virtual Bool_t IsConstructor() const
      {
         return kTRUE;
      }
      Bool_t IsDefaultCtor() const
      {
         // return kTRUE if this is the default constructor
         return IsCalled("default_ctor");
      }
      Bool_t IsCopyCtor() const
      {
         return fCopyCtor;
      }
      void SetCopyCtor(Bool_t c = kTRUE)
      {
         fCopyCtor = c;
      }
      void SetBaseClassArgument(Int_t i)
      {
         // Declare that i-th argument of ctor should be passed to base class ctor
         fFields.SetValue(Form("Arg_%d_baseclass", i), 1);
      }
      void SetMemberVariableNameForArgument(Int_t i, const Char_t* memvar)
      {
         // Store name of member variable corresponding to argument i
         fFields.SetValue(Form("Arg_%d_memvar", i), memvar);
      }

      ClassDef(KVClassConstructor, 1) //KVClassFactory helper class - description of constructor
   };

protected:
   void AddMemberInitialiserConstructor(KVClassConstructor* = nullptr);

public:
   /**
     \class KVClassDestructor
     \brief Destructor for a class generated with KVClassFactory
    */

   class KVClassDestructor : public KVClassMethod {

   public:
      KVClassDestructor()
         : KVClassMethod(kTRUE)
      {
         SetName("destructor");
         SetMethodBody("   // Destructor");
      }
      virtual ~KVClassDestructor() {}
      virtual Bool_t IsDestructor() const
      {
         return kTRUE;
      }

      ClassDef(KVClassDestructor, 1) //KVClassFactory helper class - description of destructor
   };

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
   KVClassConstructor* AddConstructor(const Char_t* argument_type = "",
                                      const Char_t* argument_name = "", const Char_t* default_value = "", const Char_t* access = "public");
   void AddAllBaseConstructors();
   void AddDefaultConstructor();
   void AddDestructor(const TString& access = "public");
   void AddMethod(const KVClassMethod& kvcm);
   void AddMethodArgument(const Char_t* method_name, const Char_t* argument_type,
                          const Char_t* argument_name = "", const Char_t* default_value = "");
   void AddMethodBody(const Char_t* method_name, const KVString& body);

   void AddHeaderIncludeFile(const Char_t* filename);
   void AddImplIncludeFile(const Char_t* filename);

   const KVList* GetListOfMethods() const
   {
      return &fMethods;
   }
   const KVList* GetListOfMembers() const
   {
      return &fMembers;
   }
   KVClassMethod* GetMethod(const Char_t* name) const
   {
      return fMethods.get_object<KVClassMethod>(name);
   }
   KVClassConstructor* GetDefaultCtor() const
   {
      return (KVClassConstructor*)GetMethod("default_ctor");
   }
   KVClassDestructor* GetDestructor() const
   {
      return (KVClassDestructor*)GetMethod("destructor");
   }

   const Char_t* GetClassName() const
   {
      return fClassName.Data();
   }
   void SetClassName(const Char_t* n)
   {
      fClassName = n;
   }
   const Char_t* GetHeaderFileName() const
   {
      // Return name of header source file
      // If path has been set with SetOutputPath() this
      // is the full path to the file
      return fClassPath != "" ? Form("%s%s.h", fClassPath.Data(), fClassName.Data()) : Form("%s.h", fClassName.Data());
   }
   const Char_t* GetImpFileName() const
   {
      // Return name of implemntation source file
      // If path has been set with SetOutputPath() this
      // is the full path to the file
      return fClassPath != "" ? Form("%s%s.cpp", fClassPath.Data(), fClassName.Data()) : Form("%s.cpp", fClassName.Data());
   }
   void SetClassDesc(const Char_t* d)
   {
      fClassDesc = d;
   }
   const Char_t* GetClassDesc() const
   {
      return fClassDesc.Data();
   }
   void SetBaseClass(const Char_t* b)
   {
      fBaseClassName = b;
      fHasBaseClass = (fBaseClassName != "");
      fBaseClassTObject = kFALSE;
      if (fHasBaseClass) {
         fBaseClass = TClass::GetClass(fBaseClassName);
         fBaseClassTObject = (fBaseClass && fBaseClass->InheritsFrom("TObject"));
      }
   }
   const Char_t* GetBaseClass() const
   {
      return fBaseClassName.Data();
   }
   Bool_t WithMultipleBaseClasses() const
   {
      return fBaseClassName.Contains(",");
   }
   Bool_t WithTemplate() const
   {
      return fWithTemplate;
   }
   Bool_t IsBaseClassTObject() const
   {
      return fBaseClassTObject;
   }
   void SetTemplate(Bool_t temp, const Char_t* temp_file);
   const Char_t* GetTemplateBase() const
   {
      return fTemplateBase;
   }
   Int_t GetNumberOfMemberVariables() const
   {
      return fMembers.GetEntries();
   }

   void Print(Option_t* opt = "") const;

   void AddGetSetMethods(const KVNameValueList&);

   void InlineAllMethods();
   void InlineAllConstructors();

   void SetOutputPath(const KVString& p)
   {
      // Set output directory for generated source files
      // Default is current working directory
      fClassPath = p;
      if (!fClassPath.EndsWith("/")) fClassPath.Append("/");
   }
   const Char_t* GetOutputPath() const
   {
      // Return output directory for generated source files
      // Default [=""] is current working directory
      return fClassPath;
   }

   void SetInheritAllConstructors(Bool_t yes = kTRUE)
   {
      // Call with kFALSE to prevent class inheriting all constructors from base class
      fInheritAllCtors = yes;
   }

   ClassDef(KVClassFactory, 5) //Factory for generating KaliVeda skeleton classes
};

#endif
