/*
$Id: KVClassFactory.cpp,v 1.19 2009/01/21 08:04:20 franklan Exp $
$Revision: 1.19 $
$Date: 2009/01/21 08:04:20 $
*/

#include "KVClassFactory.h"
#include "TSystem.h"
#include "TSystemDirectory.h"
#include "KVBase.h"
#include "TObjString.h"
#include "KVHashList.h"
#include "TClass.h"
#include "TMethod.h"
#include "TMethodArg.h"

using namespace std;

ClassImp(KVClassFactory)

KVString __add_indented_line(const KVString& line, const KVString& indent, Bool_t newline = kTRUE)
{
   KVString output;
   line.Begin("\n");
   while (!line.End()) {
      KVString _line = line.Next();
      output += (indent + _line);
      output += "\n";
   }
   if (!newline) output.Remove(output.Length() - 1);
   return output;
}

KVClassFactory::KVClassFactory()
   : fBaseClass(nullptr), fHasBaseClass(kFALSE), fBaseClassTObject(kFALSE), fInlineAllMethods(kFALSE), fInlineAllCtors(kFALSE),
     fInheritAllCtors(kTRUE)
{
   // Default ctor
}

KVClassFactory::KVClassFactory(const Char_t* classname,
                               const Char_t* classdesc,
                               const Char_t* base_class,
                               Bool_t withTemplate,
                               const Char_t* templateFile)
   : fBaseClass(nullptr), fHasBaseClass(kFALSE), fBaseClassTObject(kFALSE), fInlineAllMethods(kFALSE), fInlineAllCtors(kFALSE),
     fInheritAllCtors(kTRUE)
{
   //Create a new class with the following characteristics:
   //
   //   classname = name of new class
   //   classdesc = short (one line) description of class
   //   base_class =  name of base class(es)* (if creating a derived class)
   //   withTemplate = kTRUE if template files for the '.h' and '.cpp' are to be used
   //   templateFile = base name of template files
   //
   //         *(if the class has several base classes, give a comma-separated list)
   //
   //Only classname and classdesc have to be given.
   //By default we create a new base class without use of template files.
   //
   //If withTemplate=kTRUE, the base name for the template files must be given.
   //We check that the template files exist. If they do not, an error message is
   //printed and this object will be made a zombie (test IsZombie() after constructor).
   //The template for the class structure is defined in a '.h' and a '.cpp' file,
   //as follows:
   //
   //      if templateFile="" (default), we expect base_class!="", and template files with names base_classTemplate.h and base_classTemplate.cpp
   //      must be present in either $TEMPLATEDIR, $HOME or $PWD directories.
   //      the dummy classname "base_classTemplate" will be replaced everywhere by 'classname'
   //
   //      if templateFile="/absolute/path/classTemplate" we use classTemplate.h & classTemplate.cpp in the given directory.
   //      the dummy classname "classTemplate" will be replaced everywhere by 'classname'
   //
   //      if templateFile="classTemplate" we look for classTemplate.h & classTemplate.cpp in $TEMPLATEDIR, $HOME or $PWD directories.
   //      the dummy classname "classTemplate" will be replaced everywhere by 'classname'

   fBaseClass = NULL;
   SetClassName(classname);
   SetClassDesc(classdesc);
   SetBaseClass(base_class);
   SetTemplate(withTemplate, templateFile);
   SetWhoWhen();

   if (!withTemplate) {
      // for untemplated classes we add default ctor & dtor straight away
      AddDefaultConstructor();
      AddDestructor();
   }
}

//___________________________________________________

void KVClassFactory::SetTemplate(Bool_t temp, const Char_t* templateFile)
{
   //If the class uses template files (temp=kTRUE), 'templateFile' is the base name used to find these files.
   //The template for the class structure is defined in a '.h' and a '.cpp' file,
   //as follows:
   //
   //      if templateFile="" (default), we expect a base class to have been given,
   //      and template files with names base_classTemplate.h and base_classTemplate.cpp
   //      must be present in either $TEMPLATEDIR, $HOME or $PWD directories.
   //      the dummy classname "base_classTemplate" will be replaced everywhere by 'classname'
   //
   //      if templateFile="/absolute/path/classTemplate" we use classTemplate.h & classTemplate.cpp in the given directory.
   //      the dummy classname "classTemplate" will be replaced everywhere by 'classname'
   //
   //      if templateFile="classTemplate" we look for classTemplate.h & classTemplate.cpp in $TEMPLATEDIR, $HOME or $PWD directories.
   //      the dummy classname "classTemplate" will be replaced everywhere by 'classname'

   fWithTemplate = temp;
   fTemplateBase = templateFile;

   if (temp) {
      //if we want to use template files for the new class,
      //we have to make sure they exist;
      if (!CheckTemplateFiles(GetBaseClass(), templateFile)) {
         //template files not found - cannot proceed
         Error("SetTemplate", "Cannot find template files %s. Object made a zombie.", templateFile);
         SetBit(TObject::kZombie);
      }
   }
}

//___________________________________________________

#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
void KVClassFactory::Copy(TObject& obj) const
#else
void KVClassFactory::Copy(TObject& obj)
#endif
{
   //Copy the state of this KVClassFactory to the one referenced by 'obj'
   TObject::Copy(obj);
   ((KVClassFactory&)obj).SetClassName(GetClassName());
   ((KVClassFactory&)obj).SetClassDesc(GetClassDesc());
   ((KVClassFactory&)obj).SetBaseClass(GetBaseClass());
   ((KVClassFactory&)obj).SetTemplate(WithTemplate(), GetTemplateBase());
   ((KVClassFactory&)obj).SetWhoWhen();
   if (fMethods.GetEntries()) {
      TIter next(&fMethods);
      KVClassMethod* meth;
      while ((meth = (KVClassMethod*)next()))((KVClassFactory&)obj).AddMethod(*meth);
   }
   if (fHeadInc.GetEntries()) fHeadInc.Copy(((KVClassFactory&)obj).fHeadInc);
   if (fImpInc.GetEntries()) fImpInc.Copy(((KVClassFactory&)obj).fImpInc);
   ((KVClassFactory&)obj).SetOutputPath(GetOutputPath());
}

KVClassFactory::KVClassFactory(const KVClassFactory& obj) : TObject()
{
   //ctor par copie
   fBaseClass = NULL;
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   obj.Copy(*this);
#else
   ((KVClassFactory&)obj).Copy(*this);
#endif
}

//___________________________________________________
void KVClassFactory::WriteWhoWhen(ofstream& file)
{
   // Write a stub doxygen comment block to be used to document the file
   // This will include
   //  class name, brief description, author name and date

   file << "/**\n \\class " << GetClassName() << "\n \\brief " << GetClassDesc() << "\n\n ";
   file << "Write a detailed documentation for your class here, see doxygen manual for help.\n\n ";
   file << "\\author " << fAuthor.Data() << "\n \\date " << fNow.AsString() << "\n*/\n\n";
}

//___________________________________________________
void KVClassFactory::WritePreProc(ofstream& file)
{
   //Write pre-processor directives in file
   //i.e. '#ifndef __TOTO_H' etc.
   //If this class has inheritance, we add '#include "base_class.h"' for each of the base classes.
   //and any other include files added using AddHeaderIncludeFile

   TString tmp = fClassName;
   tmp.ToUpper();
   tmp.Prepend("__");
   tmp.Append("_H");
   file << "#ifndef " << tmp.Data() << endl;
   file << "#define " << tmp.Data() << "\n" << endl;
   //base class(es) ?
   if (fHasBaseClass) {
      if (WithMultipleBaseClasses()) {
         fBaseClassName.Begin(",");
         while (!fBaseClassName.End())
            file << "#include \"" << fBaseClassName.Next(kTRUE) << ".h\"" << endl;
         file << endl;
      }
      else
         file << "#include \"" << fBaseClassName.Data() << ".h\"\n" << endl;
   }
   if (fHeadInc.GetSize()) {
      TIter next(&fHeadInc);
      TObjString* str;
      while ((str = (TObjString*)next())) {
         file << "#include \"" << str->String().Data() << "\"\n" << endl;
      }
   }
}

//___________________________________________________
void KVClassFactory::WriteClassDec(ofstream& file)
{
   // Write the class declaration in the header file
   // This will include a default constructor & destructor and
   // any methods added using AddMethod, sorted according to
   // their access type.

   file << "class " << fClassName.Data();
   if (fHasBaseClass) {
      file << " : ";
      if (WithMultipleBaseClasses()) {
         fBaseClassName.Begin(",");
         file << "public " << fBaseClassName.Next(kTRUE);
         while (!fBaseClassName.End()) file << ", public " << fBaseClassName.Next(kTRUE);
      }
      else
         file << "public " << fBaseClassName.Data();
   }
   file << "\n{" << endl;

   // private members
   unique_ptr<KVSeqCollection> prem(fMembers.GetSubListWithMethod("private", "GetAccess"));
   if (prem->GetEntries()) {
      file << "\n" << endl;
      KVString line;
      TIter next(prem.get());
      KVClassMember* meth;
      while ((meth = (KVClassMember*)next())) {
         meth->WriteDeclaration(line);
         file << "   " << line.Data() << endl;
      }
   }

   // private methods + constructors + destructor (if private)
   unique_ptr<KVSeqCollection> priv(fMethods.GetSubListWithMethod("private", "GetAccess"));
   unique_ptr<KVSeqCollection> ctor(priv->GetSubListWithMethod("1", "IsConstructor"));
   if (ctor->GetEntries()) {
      file << endl;
      KVString line;
      TIter next(ctor.get());
      KVClassMethod* meth;
      while ((meth = (KVClassMethod*)next())) {
         meth->WriteDeclaration(line);
         file << "   " << line.Data() << endl;
      }
   }
   if (priv->GetEntries()) {
      file << endl;
      KVString line;
      TIter next(priv.get());
      KVClassMethod* meth;
      while ((meth = (KVClassMethod*)next())) {
         if (meth->IsNormalMethod()) {
            meth->WriteDeclaration(line);
            file << "   " << line.Data() << endl;
         }
      }
   }
   // private destructor
   if (GetDestructor()->IsPrivate()) {
      KVString line;
      GetDestructor()->WriteDeclaration(line);
      file << "   " << line.Data() << endl;
   }

   bool protected_written = kFALSE;

   // protected members
   prem.reset(fMembers.GetSubListWithMethod("protected", "GetAccess"));
   if (prem->GetEntries()) {
      file << "\nprotected:" << endl;
      protected_written = kTRUE;
      KVString line;
      TIter next(prem.get());
      KVClassMember* meth;
      while ((meth = (KVClassMember*)next())) {
         meth->WriteDeclaration(line);
         file << "   " << line.Data() << endl;
      }
   }

   // protected methods + ctors + dtor (if protected)
   unique_ptr<KVSeqCollection> prot(fMethods.GetSubListWithMethod("protected", "GetAccess"));
   ctor.reset(prot->GetSubListWithMethod("1", "IsConstructor"));
   if (ctor->GetEntries()) {
      if (!protected_written) {
         file << "\nprotected:" << endl;
         protected_written = kTRUE;
      }
      else
         file << endl;
      KVString line;
      TIter next(ctor.get());
      KVClassMethod* meth;
      while ((meth = (KVClassMethod*)next())) {
         meth->WriteDeclaration(line);
         file << "   " << line.Data() << endl;
      }
   }
   if (prot->GetEntries()) {
      if (!protected_written) {
         file << "\nprotected:" << endl;
         protected_written = kTRUE;
      }
      else
         file << endl;
      KVString line;
      TIter next(prot.get());
      KVClassMethod* meth;
      while ((meth = (KVClassMethod*)next())) {
         if (meth->IsNormalMethod()) {
            meth->WriteDeclaration(line);
            file << "   " << line.Data() << endl;
         }
      }
   }
   // protected destructor
   if (GetDestructor()->IsProtected()) {
      if (!protected_written) {
         file << "\nprotected:" << endl;
         protected_written = kTRUE;
      }
      else
         file << endl;
      KVString line;
      GetDestructor()->WriteDeclaration(line);
      file << "   " << line.Data() << endl;
   }

   //public methods
   file << "public:" << endl;
   //list of public ctors
   unique_ptr<KVSeqCollection> pub(fMethods.GetSubListWithMethod("public", "GetAccess"));
   ctor.reset(pub->GetSubListWithMethod("1", "IsConstructor"));
   unique_ptr<KVSeqCollection> pubnor(pub->GetSubListWithMethod("1", "IsNormalMethod"));
   if (ctor->GetEntries()) {
      KVString line;
      TIter next(ctor.get());
      KVClassMethod* meth;
      while ((meth = (KVClassMethod*)next())) {
         meth->WriteDeclaration(line);
         file << line.Data() << endl;
      }
   }
   // public destructor
   if (GetDestructor()->IsPublic()) {
      file << endl;
      KVString line;
      GetDestructor()->WriteDeclaration(line);
      file << line.Data() << endl;
      if (pubnor->GetEntries()) file << endl;
   }

   // public methods
   if (pubnor->GetEntries()) {
      KVString line;
      TIter next(pubnor.get());
      KVClassMethod* meth;
      while ((meth = (KVClassMethod*)next())) {
         meth->WriteDeclaration(line);
         file << line.Data() << endl;
      }
   }

   file << "\n   ClassDef(" << fClassName.Data() << ",1)//";
   file << fClassDesc.Data() << endl;
   file << "};\n\n#endif" << endl;      //don't forget to close the preprocessor #if !!!
}

//___________________________________________________
void KVClassFactory::WriteClassImp()
{
   //Write the class implementation file
   //This includes a class description in pure HTML

   ofstream file_cpp;

   // make sure any required directories exist
   if (fClassPath != "") gSystem->mkdir(fClassPath, kTRUE);
   file_cpp.open(GetImpFileName());

   file_cpp << "#include \"" << fClassName.Data() << ".h\"" << endl;
   if (fImpInc.GetSize()) {
      TIter next(&fImpInc);
      TObjString* str;
      while ((str = (TObjString*)next())) {
         file_cpp << "#include \"" << str->String().Data() << "\"" << endl;
      }
   }
   file_cpp << endl << "ClassImp(" << fClassName.Data() << ")\n" << endl << endl;

   // list of constructors
   unique_ptr<KVSeqCollection> ctor(fMethods.GetSubListWithMethod("1", "IsConstructor"));
   if (ctor->GetEntries()) {
      KVString line;
      TIter next(ctor.get());
      KVClassMethod* meth;
      while ((meth = (KVClassMethod*)next())) {
         if (!meth->IsInline()) {
            meth->WriteImplementation(line);
            file_cpp << line.Data() << endl;
         }
      }
   }

   // write destructor
   if (!GetDestructor()->IsInline()) {
      KVString line;
      GetDestructor()->WriteImplementation(line);
      file_cpp << line.Data() << endl;
   }

   //write implementations of added methods
   unique_ptr<KVSeqCollection> normeth(fMethods.GetSubListWithMethod("1", "IsNormalMethod"));
   if (normeth->GetEntries()) {
      KVString line;
      TIter next(normeth.get());
      KVClassMethod* meth;
      while ((meth = (KVClassMethod*)next())) {
         if (!meth->IsInline()) {
            meth->WriteImplementation(line);
            file_cpp << line.Data() << endl << endl;
         }
      }
   }

   file_cpp.close();

   cout << "<KVClassFactory::WriteClassImp> : File " << GetImpFileName() << " generated." << endl;
}

//___________________________________________________
void KVClassFactory::WriteClassHeader()
{
   //Write the class header file

   ofstream file_h;

   // make sure any required directories exist
   if (fClassPath != "") gSystem->mkdir(fClassPath, kTRUE);
   file_h.open(GetHeaderFileName());

   WritePreProc(file_h);
   WriteWhoWhen(file_h);
   WriteClassDec(file_h);

   file_h.close();

   cout << "<KVClassFactory::WriteClassHeader> : File " << GetHeaderFileName() << " generated." << endl;
}

//___________________________________________________
void KVClassFactory::MakeClass(const Char_t* classname,
                               const Char_t* classdesc,
                               const Char_t* base_class,
                               Bool_t withTemplate,
                               const Char_t* templateFile)
{
   //Static method for generating skeleton header and implementation files for a new class.
   //
   //Give a name for the class and a short description, used for HTML doc.
   //
   //The optional string 'base_class' gives the name(s) of the parent class(es)*, in case of inheritance;
   //if not given, the new class will be a base class.
   //    *(in case of several base classes, give a comma-separated list)
   //
   //if withTemplate=kTRUE, we use a template for the class structure, defined in a '.h' and a '.cpp' file,
   //as follows:
   //
   //    - if templateFile="" (default), we expect base_class!="", and template files with names base_classTemplate.h and base_classTemplate.cpp
   //      must be present in either $KVROOT/KVFiles, $HOME or $PWD directories.
   //      the dummy classname "base_classTemplate" will be replaced everywhere by 'classname'
   //
   //    - if templateFile="/absolute/path/classTemplate" we use classTemplate.h & classTemplate.cpp in the given directory.
   //      the dummy classname "classTemplate" will be replaced everywhere by 'classname'
   //
   //    - if templateFile="classTemplate" we look for classTemplate.h & classTemplate.cpp in $KVROOT/KVFiles, $HOME or $PWD directories.
   //      the dummy classname "classTemplate" will be replaced everywhere by 'classname'
   //
   //Example of use:
   //~~~~~{.cpp}
   //      KVClassFactory::MakeClass("MyClass", "A brand new class", "TObject")
   //~~~~~
   //will generate the following MyClass.h and MyClass.cpp files:
   //~~~~{.cpp}
//MyClass.h ======================================>>>>
// //Created by KVClassFactory on Fri Mar 24 23:52:54 2006    (<-------creation date)
// //Author: John Frankland    (<----- full name of user who calls MakeClass method)
//
// #ifndef __MYCLASS_H
// #define __MYCLASS_H
//
// #include <TObject.h>    (<----- header file for parent class, if necessary)
//
// class MyClass : public TObject
// {
//      public:
//
//      MyClass();
//      virtual ~MyClass();
//
//      ClassDef(MyClass,1)//A brand new class    (<------- class description)
// };
//
// #endif
//<<<<<<<===========================================
//
   //~~~~
   //~~~~{.cpp}
//MyClass.cpp=================================>>>>>>>
// //Created by KVClassFactory on Fri Mar 24 23:52:54 2006
// //Author: John Frankland   (<----- full name of user who calls MakeClass method)
//
// #include "MyClass.h"
//
// ClassImp(MyClass)
//
// ////////////////////////////////////////////////////////////////////////////////
// // A brand new class    (<------- class description)
// ////////////////////////////////////////////////////////////////////////////////
//
// MyClass::MyClass()
// {
//      //Default constructor
// }
//
// MyClass::~MyClass()
// {
//      //Destructor
// }
   //~~~~

   KVClassFactory cf(classname, classdesc, base_class, withTemplate, templateFile);
   if (!cf.IsZombie()) cf.GenerateCode();
}

void KVClassFactory::AddDefaultConstructor()
{
   if (!GetDefaultCtor()) {
      KVClassConstructor* ctor(AddConstructor());// add default ctor
      ctor->SetMethodComment("Default constructor");
   }
}

void KVClassFactory::GenerateCode()
{
   // Generate header and implementation file for currently-defined class

   if (IsZombie()) {
      Warning("GenerateCode", "Object is zombie. No code will be generated.");
      return;
   }

   InlineAllConstructors();

   if (fWithTemplate) {
      WriteClassWithTemplateHeader();
      WriteClassWithTemplateImp();
   }
   else {
      AddDefaultConstructor();
      AddDestructor();
      if (fInheritAllCtors) AddAllBaseConstructors();
      AddCopyConstructor();
      AddMemberInitialiserConstructor();
      AddAssignmentOperator();
      if (fBaseClassTObject) {
         // check for base class which inherits from TObject
         // if so, we add a skeleton Copy(const TObject&) method
         // and use it in the copy ctor
         AddTObjectCopyMethod();
      }
      else if (!fHasBaseClass) {
         // if a base class is defined, even if it does not inherit TObject,
         // we assume that it functions correctly with the ROOT CLI.
         // otherwise, we need to add #include "Rtypes.h" for definition of ClassDef and ClassImp
         AddHeaderIncludeFile("Rtypes.h");
      }
      GenerateGettersAndSetters();
      if (fInlineAllMethods) InlineAllMethods();
      if (fInlineAllCtors) InlineAllConstructors();

      WriteClassHeader();
      WriteClassImp();
   }
}

KVClassFactory::KVClassMember* KVClassFactory::AddMember(const Char_t* name, const Char_t* type, const Char_t* comment, const Char_t* access)
{
   // Add a member variable to the class, with name 'f[name]' according to ROOT convention.
   // The access type is by default "protected", in accordance with OO-encapsulation.

   KVClassMember* m = new KVClassMember(name, type, comment, access);
   fMembers.Add(m);
   return m;
}

void KVClassFactory::GenerateGettersAndSetters()
{
   // for each member variable we generate inlined Get/Set methods
   // for boolean members we generate Is/SetIs methods

   if (GetNumberOfMemberVariables()) {
      TIter it(&fMembers);
      KVClassMember* m;
      while ((m = (KVClassMember*)it())) {
         TString mem_name = m->GetRealName();
         TString mem_type = m->GetType();
         KVClassMethod* meth;
         if (mem_type == "Bool_t") meth = AddMethod(Form("SetIs%s", mem_name.Data()), "void");
         else meth = AddMethod(Form("Set%s", mem_name.Data()), "void");
         meth->SetInline();
         meth->AddArgument(mem_type);
         meth->SetMethodComment(
            Form("Set value of %s", m->GetName())
         );
         meth->SetMethodBody(
            Form("   %s=arg1;", m->GetName())
         );
         if (mem_type == "Bool_t") meth = AddMethod(Form("Is%s", mem_name.Data()), mem_type, "public", kFALSE, kTRUE);
         else meth = AddMethod(Form("Get%s", mem_name.Data()), mem_type, "public", kFALSE, kTRUE);
         meth->SetInline();
         meth->SetMethodComment(
            Form("Get value of %s", m->GetName())
         );
         meth->SetMethodBody(
            Form("   return %s;", m->GetName())
         );
      }
   }
}

//___________________________________________________
void KVClassFactory::SetWhoWhen()
{
   //Set date, time and user name based on current environment
   //If available, we use the full name of the current user, otherwise we use the login name.

   UserGroup_t* user = gSystem->GetUserInfo();
   fAuthor = user->fRealName;
   if (fAuthor == "")
      fAuthor = user->fUser;
   // remove trailing commas (???)
   if (fAuthor.EndsWith(",")) {
      Ssiz_t len = fAuthor.Length() - 1;
      do {
         fAuthor.Remove(len);
         --len;
      }
      while (fAuthor.EndsWith(","));
   }
   delete user;
   fNow.Set();
}

//___________________________________________________

Ssiz_t KVClassFactory::FindNextUncommentedLine(TString& file, Ssiz_t beg)
{
   // Return position of beginning of next uncommented line in file starting
   // from position beg (default = 0).
   // An uncommented line is a line outside of a "/* ... */" block
   // containing non-whitespace material before any '/*' or '//'.
   // If no uncommented line is found we return -1.

   Ssiz_t pos = beg;
   Ssiz_t beg_cur_line = beg;
   Bool_t nonWS = kFALSE;

   while (pos < file.Length()) {

      if (file(pos) == ' ' || file(pos) == '\t') {
         // skip whitespace & tab
         pos++;
      }
      else if (file(pos) == '\n') {
         // reached newline without seeing comment
         // if there is non-whitespace stuff on line, this is OK
         if (nonWS) return beg_cur_line;
         pos++;
         beg_cur_line = pos;
      }
      else if (file(pos) == '/') {

         if (file(pos + 1) == '*') {
            // beginning of comment block
            // was there non-whitespace stuff before this ?
            if (nonWS) return beg_cur_line;
            Ssiz_t len = file.Index("*/", pos + 2); // find end of comment
            if (len > -1) pos = len + 2;
            else {
               // end of comment block not found in rest of file
               // return -1
               pos = -1;
               break;
            }
         }
         else if (file(pos + 1) == '/') {
            // beginning of 1-line comment
            // was there non-whitespace stuff before this ?
            if (nonWS) return beg_cur_line;
            Ssiz_t len = file.Index('\n', pos + 2); // find end of line
            if (len > 0) {
               pos = len + 1;
               beg_cur_line = pos;
            }
            else {
               // end of comment not found in rest of file
               // return -1
               pos = -1;
               break;
            }
         }
      }
      else {
         //non-whitespace stuff
         nonWS = kTRUE;
         pos++;
      }
   }
   // no new line at end of first uncommented file which is also the last line in the file
   if (nonWS && pos > -1) return beg_cur_line;
   return pos;
}

//___________________________________________________
void KVClassFactory::WriteClassWithTemplateHeader()
{
   //Writes the header file for a class using a template file.
   //The ClassDef line in the template file will be replaced with one corresponding to the new class.

   ofstream file_h;

   // make sure any required directories exist
   if (fClassPath != "") gSystem->mkdir(fClassPath, kTRUE);
   file_h.open(GetHeaderFileName());

   WritePreProc(file_h);
   WriteWhoWhen(file_h);

   ifstream file_h_template;

   //open file whose full path was stored in fTemplateH
   if (!KVBase::SearchAndOpenKVFile(fTemplateH.Data(), file_h_template)) {
      //this should never happen!
      cout <<
           "<KVClassFactory::WriteClassWithTemplateHeader>: cannot open " <<
           fTemplateH.Data() << endl;
      return;
   }

   TString headFile;
   headFile.ReadFile(file_h_template);
   file_h_template.close();

   //find ClassDef
   Ssiz_t class_ind;
   if ((class_ind = headFile.Index("ClassDef")) > -1) {

      // find end of line just before ClassDef
      Ssiz_t end_of_line = class_ind;
      while (headFile(--end_of_line) != '\n') ;
      //cut file into two parts: before ClassDef line, after ClassDef line
      TString part1 = headFile(0, end_of_line + 1);
      //keep part of file from "ClassDef" onwards.
      TString subs = headFile(class_ind, headFile.Length() - 1);
      //find next newline character i.e. the end of ClassDef line
      Ssiz_t nl = subs.Index("\n");
      TString part3 = subs(nl + 1, subs.Length() - 1);
      //write ClassDef line
      TString part2 = "   ClassDef(";
      part2 += fClassName;
      part2 += ",1)//";
      part2 += fClassDesc;
      part2 += "\n";

      KVString part_add;
      //write declarations of added methods
      // group by access type
      if (fMethods.GetSize()) {
         KVString line;
         KVString acc_types[] = {"private", "protected", "public", "-"};
         int iac = 0;
         while (acc_types[iac] != "-") {
            std::unique_ptr<KVSeqCollection> sublist(fMethods.GetSubListWithLabel(acc_types[iac]));
            if (sublist->GetEntries()) {
               part_add += acc_types[iac];
               part_add += ":\n";
               TIter next(sublist.get());
               KVClassMethod* meth;
               while ((meth = (KVClassMethod*)next())) {
                  meth->WriteDeclaration(line);
                  part_add += line;
                  part_add += "\n";
               }
               part_add += "\n";
            }
            ++iac;
         }
      }

      //stick all parts together
      headFile = part1 + part_add + part2 + part3;
   }

   file_h << headFile.ReplaceAll(fTemplateClassName.Data(),
                                 fClassName.Data());
   file_h << "\n#endif" << endl;      //don't forget to close the preprocessor #if !!!
   file_h.close();

   cout << "<KVClassFactory::WriteClassWithTemplateHeader> : File " << GetHeaderFileName() << " generated." << endl;

}

//___________________________________________________
void KVClassFactory::WriteClassWithTemplateImp()
{
   // Writes the implementation file for the class

   ofstream file_cpp;

   // make sure any required directories exist
   if (fClassPath != "") gSystem->mkdir(fClassPath, kTRUE);
   file_cpp.open(GetImpFileName());

   file_cpp << "#include \"" << fClassName.Data() << ".h\"" << endl;
   if (fImpInc.GetSize()) {
      TIter next(&fImpInc);
      TObjString* str;
      while ((str = (TObjString*)next())) {
         file_cpp << "#include \"" << str->String().Data() << "\"" << endl;
      }
   }
   file_cpp << endl << "ClassImp(" << fClassName.Data() << ")\n" << endl;

   TString cppFile;
   ifstream file_cpp_template;

   //open file whose full path was stored in fTemplateCPP
   if (!KVBase::
         SearchAndOpenKVFile(fTemplateCPP.Data(), file_cpp_template)) {
      //this should never happen!
      cout << "<KVClassFactory::WriteClassWithTemplateImp>: cannot open "
           << fTemplateCPP.Data() << endl;
      return;
   }

   cppFile.ReadFile(file_cpp_template);
   file_cpp_template.close();
   file_cpp << cppFile.ReplaceAll(fTemplateClassName.Data(),
                                  fClassName.Data());

   //write implementations of added methods
   if (fMethods.GetSize()) {
      KVString line;
      TIter next(&fMethods);
      KVClassMethod* meth;
      while ((meth = (KVClassMethod*)next())) {
         meth->WriteImplementation(line);
         file_cpp << line.Data();
      }
   }
   file_cpp.close();

   cout << "<KVClassFactory::WriteClassWithTemplateImp> : File " << GetImpFileName() << " generated." << endl;
}

//______________________________________________________

Bool_t KVClassFactory::CheckTemplateFiles(const Char_t* base_class,
      const Char_t* templateFile)
{
   //Check that we have the necessary template files to generate the new class.
   //
   //      if templateFile="" (default), we expect base_class!="", and template files with names base_classTemplate.h and base_classTemplate.cpp
   //      must be present in either $TEMPLATEDIR, $HOME or $PWD directories.
   //      the dummy classname "base_classTemplate" will be replaced everywhere by the new class name
   //
   //      if templateFile="/absolute/path/classTemplate" we use classTemplate.h & classTemplate.cpp in the given directory.
   //      the dummy classname "classTemplate" will be replaced everywhere by the new class name
   //
   //      if templateFile="classTemplate" we look for classTemplate.h & classTemplate.cpp in $TEMPLATEDIR, $HOME or $PWD directories.
   //      the dummy classname "classTemplate" will be replaced everywhere by the new class name
   //
   //if all goes well, this method returns kTRUE and fTemplateClassName contains the dummy class name
   //which should be replaced in the template files with the name of the new class, while fTemplateH and fTemplateCPP
   //contain the full paths to the template files.

   if (strcmp(base_class, "")) {
      //we have a base class name.
      //the dummy class name is base_classTemplate and we look for base_classTemplate.h and base_classTemplate.cpp
      fTemplateClassName = base_class;
      fTemplateClassName += "Template";
   }
   else {
      //no base class. we look for templateFile.h and templateFile.cpp
      fTemplateClassName = gSystem->BaseName(templateFile);
   }

   TString filename = fTemplateClassName + ".h";
   //we look for the template .h file in $TEMPLATEDIR, in $HOME and in $PWD
   //if found, fTemplateH contains the full path to the file.
   //if not found, we return kFALSE as we cannot proceed with the class generation.

   if (!KVBase::SearchKVFile(KVBase::GetTEMPLATEDIRFilePath(filename), fTemplateH)) {
      if (!KVBase::SearchKVFile(filename, fTemplateH)) return kFALSE;
   }
   filename = fTemplateClassName + ".cpp";
   //same treatment for '.cpp' file
   if (!KVBase::SearchKVFile(KVBase::GetTEMPLATEDIRFilePath(filename), fTemplateCPP)) {
      if (!KVBase::SearchKVFile(filename.Data(), fTemplateCPP)) return kFALSE;
   }

   return kTRUE;
}

//______________________________________________________

void KVClassFactory::AddMethod(const KVClassMethod& kvcm)
{
   //A new KVClassMethod object will be created and added to the class,
   //copying the informations held in kvcm

   KVClassMethod* meth = new KVClassMethod(kvcm);
   fMethods.Add(meth);
}

//______________________________________________________

KVClassFactory::KVClassMethod* KVClassFactory::AddMethod(const Char_t* name, const Char_t* return_type, const Char_t* access,
      Bool_t isVirtual, Bool_t isConst)
{
   // Add a method to the class.
   // User must give return type and name of method.
   // Optional arguments determine access type (public, protected or private)
   // and if the method is 'virtual' and/or 'const'
   //
   // If another method with the same name already exists, user should keep the returned
   // pointer to the new KVClassMethod object and use KVClassMethod::AddArgument(),
   // KVClassMethod::SetMethodBody() in order to define arguments, method body, etc.
   // instead of using the AddMethodArgument, AddMethodBody methods

   KVClassMethod* meth = new KVClassMethod;
   fMethods.Add(meth);
   meth->SetName(name);
   meth->SetClassName(fClassName);
   meth->SetReturnType(return_type);
   meth->SetAccess(access);
   meth->SetVirtual(isVirtual);
   meth->SetConst(isConst);
   return meth;
}

//______________________________________________________

KVClassFactory::KVClassConstructor* KVClassFactory::AddConstructor(const Char_t* argument_type,
      const Char_t* argument_name, const Char_t* default_value, const Char_t* access)
{
   // Add a constructor with or without arguments to the class.
   // If no arguments are given, adds a default constructor (no arguments).
   //
   // Optional argument 'access' determines access type (public, protected or private)    [default: "public"]
   //
   // If more than one argument is needed, user should keep the returned pointer
   // to the new object and use KVClassMethod::AddArgument() in order to add further arguments.
   //
   // In order to define the implementation of the ctor method, user should keep the returned pointer
   // to the new object and use KVClassMethod::SetMethodBody(KVString&).

   KVClassConstructor* meth = new KVClassConstructor(this);
   fMethods.Add(meth);
   if (strcmp(argument_type, "")) {
      // use type of first argument as name of ctor method
      meth->SetName(argument_type);
      meth->AddArgument(argument_type, argument_name, default_value);
   }
   else {
      // default constructor
      meth->SetName("default_ctor");
   }
   meth->SetClassName(fClassName);
   if (fHasBaseClass) meth->SetBaseClass(fBaseClassName);
   meth->SetAccess(access);
   return meth;
}

//______________________________________________________

void KVClassFactory::AddMethodArgument(const Char_t* method_name, const Char_t* argument_type,
                                       const Char_t* argument_name, const Char_t* default_value)
{
   //Add an argument to the method 'method_name' added to the class using AddMethod.
   //User must give type of argument.
   //Optional argument argument_name gives name of argument (will be used in implementation declaration).
   //Optional argument default_value gives default value.

   KVClassMethod* meth = (KVClassMethod*)fMethods.FindObjectByName(method_name);
   if (!meth) {
      Error("AddMethodArgument",
            "Method %s not found.", method_name);
      return;
   }
   meth->AddArgument(argument_type, argument_name, default_value);
}

//______________________________________________________

void KVClassFactory::AddMethodBody(const Char_t* method_name, const KVString& body)
{
   //Set the body of the code for method 'method_name' added to the class using AddMethod.
   //N.B. does not work for implementing constructors, see AddConstructor

   KVClassMethod* meth = (KVClassMethod*)fMethods.FindObjectByName(method_name);
   if (!meth) {
      Error("AddMethodBody",
            "Method %s not found.", method_name);
      return;
   }
   meth->SetMethodBody(body);
}

void KVClassFactory::AddMethodComment(const Char_t* method_name, const KVString& comment)
{
   //Set the comments for method 'method_name' added to the class using AddMethod.

   KVClassMethod* meth = (KVClassMethod*)fMethods.FindObjectByName(method_name);
   if (!meth) {
      Error("AddMethodComment",
            "Method %s not found.", method_name);
      return;
   }
   meth->SetMethodComment(comment);
}

//______________________________________________________

void KVClassFactory::AddHeaderIncludeFile(const Char_t* filename)
{
   //Add a file which will appear in the 'includes' list of the '.h' file
   //i.e. we will add a line
   //   #include "filename"
   //to the .h file
   fHeadInc.Add(new TObjString(filename));
}

//______________________________________________________

void KVClassFactory::AddImplIncludeFile(const Char_t* filename)
{
   //Add a file which will appear in the 'includes' list of the '.cpp' file
   //i.e. we will add a line
   //   #include "filename"
   //to the .cpp file
   fImpInc.Add(new TObjString(filename));
//   Info("AddImplIncludeFile", "Called for %s %#x. List now contains %d filenames.",
//         ClassName(), (long)this, fImpInc.GetEntries());
}

//______________________________________________________

void KVClassFactory::Print(Option_t*) const
{
   //Print infos on object
   Info("Print", "object name = %s, address = %p", GetName(), this);
   cout << " * fClassName = " << fClassName.Data() << endl;
   cout << " * fClassDesc = " << fClassDesc.Data() << endl;
   cout << " * fBaseClass = " << fBaseClassName.Data() << endl;
   cout << " * fTemplateBase = " << fTemplateBase.Data() << endl;
   cout << "---------> Methods" << endl;
   fMethods.Print();
   cout << "---------> Header Includes" << endl;
   fHeadInc.Print();
   cout << "---------> Implementation Includes" << endl;
   fImpInc.Print();
}

void KVClassFactory::AddGetSetMethods(const KVNameValueList& nvl)
{
   // For each named parameter in the list, we add protected member variables with the name and type of the parameter.

   int npars = nvl.GetNpar();
   for (int i = 0; i < npars; i++) {
      KVNamedParameter* par = nvl.GetParameter(i);
      if (par->IsString()) {
         AddMember(par->GetName(), "TString", "member automatically generated by KVClassFactory::AddGetSetMethods");
         // make sure #include "TString.h" appears in header file
         if (! fHeadInc.FindObject("TString.h")) AddHeaderIncludeFile("TString.h");
      }
      else if (par->IsInt()) {
         AddMember(par->GetName(), "Int_t", "member automatically generated by KVClassFactory::AddGetSetMethods");
      }
      else if (par->IsDouble()) {
         AddMember(par->GetName(), "Double_t", "member automatically generated by KVClassFactory::AddGetSetMethods");
      }
      else if (par->IsBool()) {
         AddMember(par->GetName(), "Bool_t", "member automatically generated by KVClassFactory::AddGetSetMethods");
      }
   }
}

void KVClassFactory::InlineAllMethods(bool yes)
{
   // \param yes =true: Write implementation of all non-ctor/non-dtor methods in the header file of the class

   TIter it(&fMethods);
   KVClassMethod* m;
   while ((m = (KVClassMethod*)it())) {
      if (!m->IsConstructor() && !m->IsDestructor()) m->SetInline(yes);
   }
   fInlineAllMethods = yes;
}

void KVClassFactory::InlineAllConstructors(bool yes)
{
   // \param yes =true: Write implementation of all constructors & the destructor in the header file of the class.

   unique_ptr<KVSeqCollection> ctors(fMethods.GetSubListWithMethod("1", "IsConstructor"));
   ctors->R__FOR_EACH(KVClassConstructor, SetInline)(yes);
   fInlineAllCtors = yes;
   KVClassDestructor* dtor = fMethods.get_object<KVClassDestructor>("destructor");
   if (dtor) dtor->SetInline(yes);
}

//__________________________________________________________________________________

void KVClassFactory::AddTObjectCopyMethod()
{
   // Adds skeleton standard ROOT Copy method if class has
   // member variables

   if (!GetNumberOfMemberVariables()) return;

   AddMethod("Copy", "void", "public", kFALSE, kTRUE);
   AddMethodArgument("Copy", "TObject&", "obj");
   KVString body("This method copies the current state of 'this' object into 'obj'\n");
   body += "You should add here any member variables, for example:\n";
   body += "(supposing a member variable ";
   body += fClassName;
   body += "::fToto)\n";
   body += "   CastedObj.fToto = fToto;\n";
   body += "or\n";
   body += "   CastedObj.SetToto( GetToto() );";
   AddMethodComment("Copy", body);
   // call Copy method for base class
   body = "\n\n   ";
   body += fBaseClassName;
   body += "::Copy(obj);\n";
   if (!fMembers.GetEntries()) body += "   //";
   else body += "   ";
   body += fClassName;
   body += "& CastedObj = (";
   body += fClassName;
   body += "&)obj;\n";
   if (fMembers.GetEntries()) {
      TIter it(&fMembers);
      KVClassMember* m;
      while ((m = (KVClassMember*)it())) {
         body += "   CastedObj.Set";
         body += m->GetRealName();
         body += "( Get";
         body += m->GetRealName();
         body += "() );\n";
      }
   }
   AddMethodBody("Copy", body);
}

void KVClassFactory::AddCopyConstructor()
{
   // Adds copy constructor if class has member variables
   // If class inherits from TObject, this just calls the Copy method

   if (!GetNumberOfMemberVariables()) return;

   KVClassConstructor* ctor = AddConstructor(Form("const %s&", fClassName.Data()), "obj");
   ctor->SetCopyCtor();
   ctor->SetMethodComment("Copy constructor");
   if (fBaseClassTObject) {
      ctor->SetMethodBody("   obj.Copy(*this);");
   }

}

void KVClassFactory::AddMemberInitialiserConstructor(KVClassConstructor* base_ctor)
{
   // For classes with member variables, add a constructor which initialises
   // the values of all member variables
   // If base_ctor is non-null, it is assumed to point to a constructor which has
   // been deduced from the base class' list of constructors; in this case we
   // add our class' members to the beginning of the list of arguments
   // so that if any of the base class ctor arguments have default values, they
   // will be at the end of the list

   if (!GetNumberOfMemberVariables()) return;

   TIter next(&fMembers);
   KVClassMember* memb = (KVClassMember*)next();
   KVClassConstructor* ctor;
   Bool_t adding_to_base = kFALSE;
   Int_t add_to_base(1);
   if (base_ctor) {
      ctor = base_ctor;
      adding_to_base = kTRUE;
      ctor->AddArgument(add_to_base, memb->GetType(), memb->GetRealName());
      ctor->SetMemberVariableNameForArgument(add_to_base, memb->GetName());
   }
   else {
      ctor = AddConstructor(memb->GetType(), memb->GetRealName());
      ctor->SetMemberVariableNameForArgument(1, memb->GetName());
   }
   int arg_num = 2;
   while ((memb = (KVClassMember*)next())) {
      if (adding_to_base) {
         ctor->AddArgument(++add_to_base, memb->GetType(), memb->GetRealName());
         ctor->SetMemberVariableNameForArgument(add_to_base, memb->GetName());
      }
      else {
         ctor->AddArgument(memb->GetType(), memb->GetRealName());
         ctor->SetMemberVariableNameForArgument(arg_num++, memb->GetName());
      }
   }
   ctor->SetMethodComment("Constructor with initial values for all member variables");
}

void KVClassFactory::AddAssignmentOperator()
{
   // Add "ClassName& operator= (const ClassName&)" method if class
   // has member variables

   if (!GetNumberOfMemberVariables()) return;

   KVClassMethod* oper = AddMethod("operator=", Form("%s&", fClassName.Data()));
   oper->AddArgument(Form("const %s&", fClassName.Data()), "other");
   oper->SetMethodComment("Assignment operator");
   KVString body = "   if(this != &other) { // check for self-assignment\n";
   if (fBaseClassTObject) {
      body += "      other.Copy(*this);\n";
   }
   else {
      if (fMembers.GetEntries()) {
         TIter it(&fMembers);
         KVClassMember* m;
         while ((m = (KVClassMember*)it())) {
            body += "      Set";
            body += m->GetRealName();
            body += "( other.Get";
            body += m->GetRealName();
            body += "() );\n";
         }
      }
   }
   body += "   }\n";
   body += "   return (*this);";
   oper->SetMethodBody(body);
}

//___________________________________________________________________________________


ClassImp(KVClassFactory::KVClassMethod)
ClassImp(KVClassFactory::KVClassMember)
/////////////////////////////////////////////////////////////////////////////////////
//Helper classes for KVClassFactory

void KVClassFactory::KVClassMethod::WriteDeclaration(KVString& decl)
{
   //Write declaration in the KVString object
   //If method is inline write method body directly after declaration

   decl = "   ";
   if (fVirtual) decl += "virtual ";
   if (IsNormalMethod()) {
      decl += GetReturnType();
      decl += " ";
      decl += GetName();
   }
   else {
      if (IsDestructor()) decl += "~";
      decl += GetClassName();
   }
   decl += "(";
   for (int i = 1; i <= fNargs; i++) {
      decl += fFields.GetStringValue(Form("Arg_%d", i));
      if (fFields.HasParameter(Form("Arg_%d_name", i))) {
         decl += " ";
         decl += fFields.GetStringValue(Form("Arg_%d_name", i));
      }
      else
         decl += Form(" arg%d", i);
      if (fFields.HasParameter(Form("Arg_%d_default", i))) {
         decl += " = ";
         decl += fFields.GetStringValue(Form("Arg_%d_default", i));
      }
      if (i < fNargs) decl += ", ";
   }
   decl += ")";
   if (fConst) decl += " const";
   if (!IsInline()) {
      decl += ";";
      return;
   }
   // write method body of inline function
   write_method_body(decl);
}

void KVClassFactory::KVClassMember::WriteDeclaration(KVString& decl)
{
   //Write declaration in the KVString object

   decl = GetType();
   decl += " ";
   decl += GetName();

   decl += ";//";
   decl += GetComment();
}

void KVClassFactory::KVClassMethod::write_method_body(KVString& decl)
{
   KVString indentation = "";
   if (IsInline()) indentation = "   ";
   decl += "\n";
   decl += __add_indented_line("{", indentation);
   if (!strcmp(GetAccess(), "private")) decl += __add_indented_line("   // PRIVATE method", indentation);
   else if (!strcmp(GetAccess(), "protected")) decl += __add_indented_line("   // PROTECTED method", indentation);
   if (fFields.HasParameter("Body")) {
      //write body of method
      decl += __add_indented_line(fFields.GetStringValue("Body"), indentation);
   }
   else {
      if (fFields.HasParameter("ReturnType") && strcmp(fFields.GetStringValue("ReturnType"), "void")) {
         KVString return_dir = "return (";
         return_dir += GetReturnType();
         return_dir += ")0;";
         __add_indented_line(return_dir, indentation);
      }
   }
   decl += __add_indented_line("}", indentation, kFALSE);
}

void KVClassFactory::KVClassConstructor::write_method_body(KVString& decl)
{
   KVString indentation = "";
   if (IsInline()) indentation = "   ";
   if ((fNargs && !IsCopyCtor()) ||
         ((IsCopyCtor() || IsDefaultCtor()) && fFields.HasParameter("BaseClass"))
         || (IsCopyCtor() && !fParentClass->IsBaseClassTObject())) {
      decl += "\n   : ";
      if (fFields.HasParameter("BaseClass")) {
         // initialise base class
         decl += fFields.GetStringValue("BaseClass");
         decl += "(";
         if (!IsCopyCtor() && fNargs) {
            Int_t base_args = 0;
            for (int i = 1; i <= fNargs; i++) {
               if (fFields.HasParameter(Form("Arg_%d_baseclass", i))) {
                  ++base_args;
                  if (fFields.HasParameter(Form("Arg_%d_name", i))) {
                     if (base_args > 1) decl += ", ";
                     decl += fFields.GetStringValue(Form("Arg_%d_name", i));
                  }
                  else {
                     if (base_args > 1) decl += ", ";
                     decl += Form("arg%d", i);
                  }
               }
            }
         }
         else if (IsCopyCtor() && !fParentClass->IsBaseClassTObject()) {
            // for a derived object not inheriting TObject we call the copy ctor of the parent class
            decl += fFields.GetStringValue("Arg_1_name");
         }
         decl += ")";
         if ((fNargs && !IsCopyCtor() && fParentClass->GetNumberOfMemberVariables())
               || (IsCopyCtor() && !fParentClass->IsBaseClassTObject() && fParentClass->GetNumberOfMemberVariables())) decl += ", ";
      }
      if (fParentClass->GetNumberOfMemberVariables()) {
         // initialise member variables
         if (!IsCopyCtor() && fNargs) {
            Int_t mem_vars = 0;
            for (int i = 1; i <= fNargs; i++) {
               if (fFields.HasStringParameter(Form("Arg_%d_memvar", i))) {
                  if (mem_vars) decl += ", ";
                  decl += fFields.GetStringValue(Form("Arg_%d_memvar", i));
                  decl += "(";
                  decl += fFields.GetStringValue(Form("Arg_%d_name", i));
                  decl += ")";
                  ++mem_vars;
               }
            }
         }
         else if ((IsCopyCtor() && !fParentClass->IsBaseClassTObject())) {
            TIter it(fParentClass->GetListOfMembers());
            KVClassMember* m;
            Int_t imem = 0;
            while ((m = (KVClassMember*)it())) {
               if (imem) decl += ", ";
               decl += m->GetName();
               decl += "( ";
               decl += fFields.GetStringValue("Arg_1_name");
               decl += ".Get";
               decl += m->GetRealName();
               decl += "() )";
               ++imem;
            }
         }
      }
   }
   decl += "\n";
   decl += __add_indented_line("{", indentation);
   if (!strcmp(GetAccess(), "private")) decl += __add_indented_line("   // PRIVATE constructor", indentation);
   else if (!strcmp(GetAccess(), "protected")) decl += __add_indented_line("   // PROTECTED constructor", indentation);
   if (fFields.HasParameter("Body")) {
      //write body of method
      decl += __add_indented_line(fFields.GetStringValue("Body"), indentation);
   }
   decl += __add_indented_line("}", indentation, kFALSE);
}

void KVClassFactory::KVClassMethod::WriteImplementation(KVString& decl)
{
   //Write skeleton implementation in the KVString object
   //All constructors call the default ctor of the base class (if defined)

   decl = "";
   if (fFields.HasParameter("Comment")) {
      // Go through comment line by line adding doxygen comment markers
      KVString comment(fFields.GetStringValue("Comment"));
      comment.Begin("\n");
      // find longest line
      Ssiz_t longest = 0;
      while (!comment.End()) {
         KVString line = comment.Next();
         if (line.Length() > longest) longest = line.Length();
      }
      longest += 4; // remember to add "/// " at beginning of each comment line
      // write "////...." as long as longest line
      while (longest--) decl += "/";
      decl += "\n";
      comment.Begin("\n");
      while (!comment.End()) {
         KVString line = comment.Next();
         decl += "/// ";
         decl += line;
         decl += "\n";
      }
      decl += "\n";
   }
   decl += GetReturnType();
   if (IsNormalMethod()) {
      decl += " ";
   }
   decl += GetClassName();
   decl += "::";
   if (IsNormalMethod())
      decl += GetName();
   else {
      if (IsDestructor()) decl += "~";
      decl += GetClassName();
   }
   decl += "(";
   for (int i = 1; i <= fNargs; i++) {
      decl += fFields.GetStringValue(Form("Arg_%d", i));
      if (fFields.HasParameter(Form("Arg_%d_name", i))) {
         decl += " ";
         decl += fFields.GetStringValue(Form("Arg_%d_name", i));
      }
      else
         decl += Form(" arg%d", i);
      if (i < fNargs) decl += ", ";
   }
   decl += ")";
   if (fConst) decl += " const";
   write_method_body(decl);
   decl += "\n\n";

}

//__________________________________________________________________________________

KVClassFactory::KVClassMember::KVClassMember(const KVClassMember& obj) : KVBase()
{
   //ctor par copie
   obj.Copy(*this);
}

//__________________________________________________________________________________

KVClassFactory::KVClassMethod::KVClassMethod(const KVClassMethod& obj) : KVClassMember()
{
   //ctor par copie
   obj.Copy(*this);
}

//__________________________________________________________________________________

void KVClassFactory::KVClassMember::Copy(TObject& obj) const
{
   //copy this to obj
   KVBase::Copy(obj);
   ((KVClassMember&)obj).SetComment(GetComment());
   ((KVClassMember&)obj).fRealName = GetRealName();
}

//__________________________________________________________________________________

void KVClassFactory::KVClassMethod::Copy(TObject& obj) const
{
   //copy this to obj
   KVClassMember::Copy(obj);
   ((KVClassMethod&)obj).SetVirtual(IsVirtual());
   ((KVClassMethod&)obj).SetConst(IsConst());
   fFields.Copy(((KVClassMethod&)obj).fFields);
   ((KVClassMethod&)obj).SetNargs(GetNargs());
   ((KVClassMethod&)obj).SetInline(IsInline());
}

void KVClassFactory::KVClassMethod::AddArgument(Int_t i, const Char_t* type, const Char_t* argname, const Char_t* defaultvalue)
{
   // Add an argument to the method, which will be in i-th position in the argument list
   // Any existing arguments with indices [i,fNargs] will first be moved to [i+1,fNargs+1]

   if (!(i > 0 && i <= fNargs + 1)) {
      Error("AddArgument(Int_t i, ...)", "Must be called with i>0 & i<=%d", fNargs + 1);
      return;
   }

   if (i == fNargs + 1) { // this is just same as call to AddArgument(type,argname,...)
      AddArgument(type, argname, defaultvalue);
      return;
   }

   for (int j = fNargs; j >= i; --j) {
      if (fFields.HasStringParameter(Form("Arg_%d", j))) {
         fFields.SetValue(Form("Arg_%d", j + 1), fFields.GetStringValue(Form("Arg_%d", j)));
         fFields.RemoveParameter(Form("Arg_%d", j));
      }
      if (fFields.HasStringParameter(Form("Arg_%d_name", j))) {
         fFields.SetValue(Form("Arg_%d_name", j + 1), fFields.GetStringValue(Form("Arg_%d_name", j)));
         fFields.RemoveParameter(Form("Arg_%d_name", j));
      }
      if (fFields.HasIntParameter(Form("Arg_%d_baseclass", j))) {
         fFields.SetValue(Form("Arg_%d_baseclass", j + 1), fFields.GetIntValue(Form("Arg_%d_baseclass", j)));
         fFields.RemoveParameter(Form("Arg_%d_baseclass", j));
      }
      if (fFields.HasStringParameter(Form("Arg_%d_default", j))) {
         fFields.SetValue(Form("Arg_%d_default", j + 1), fFields.GetStringValue(Form("Arg_%d_default", j)));
         fFields.RemoveParameter(Form("Arg_%d_default", j));
      }
   }

   KVString _type(type);
   fFields.SetValue(Form("Arg_%d", i), _type);
   if (strcmp(defaultvalue, "")) {
      KVString _s(defaultvalue);
      fFields.SetValue(Form("Arg_%d_default", i), _s);
   }
   if (strcmp(argname, "")) {
      KVString _s(argname);
      fFields.SetValue(Form("Arg_%d_name", i), _s);
   }

   ++fNargs;
}

//__________________________________________________________________________________
void KVClassFactory::KVClassMethod::Print(Option_t*) const
{
   // print the KVClass method
   cout << "KVClassMethod object -----> " << GetName() << endl;
   fFields.Print();
   cout << "This method is " << GetAccess() << endl;
   if (fConst) cout << "This method is CONST" << endl;
   if (fVirtual) cout << "This method is VIRTUAL" << endl;
}

//__________________________________________________________________________________
void KVClassFactory::KVClassMember::Print(Option_t*) const
{
   // print the KVClass member
   cout << "KVClassMember object -----> " << GetType() << " " << GetName() << endl;
   cout << "This member is " << GetAccess() << endl;
}

//__________________________________________________________________________________

void KVClassFactory::AddAllBaseConstructors()
{
   // Add constructors with the same signature as all base class constructors
   // (apart from the default ctor or any copy constructors, which are a special case)
   // If this class has its own member variables, we add them to the argument list
   // of each base class constructor.

   if (!fBaseClass) return;

   KVHashList clist;
   clist.AddAll(fBaseClass->GetListOfMethods());
   unique_ptr<KVSeqCollection> constructors(clist.GetSubListWithName(fBaseClassName));
   TIter next_ctor(constructors.get());
   TMethod* method;
   while ((method = (TMethod*)next_ctor())) {
      if (!method->GetNargs()) continue; // ignore default ctor
      TList* args = method->GetListOfMethodArgs();
      TMethodArg* arg = (TMethodArg*)args->First();
      TString typenam = arg->GetFullTypeName();
      if (typenam.Contains(fBaseClassName)) continue; // ignore copy ctor
      KVClassConstructor* ctor;
      if (arg->GetDefault()) ctor = AddConstructor(typenam, arg->GetName(), arg->GetDefault());
      else ctor = AddConstructor(typenam, arg->GetName());
      ctor->SetBaseClassArgument(1);
      for (int i = 1; i < method->GetNargs(); i++) {
         arg = (TMethodArg*)args->At(i);
         if (arg->GetDefault()) ctor->AddArgument(arg->GetFullTypeName(), arg->GetName(), arg->GetDefault());
         else ctor->AddArgument(arg->GetFullTypeName(), arg->GetName());
         ctor->SetBaseClassArgument(i + 1);
      }
      ctor->SetMethodComment(Form("Constructor inherited from %s", GetBaseClass()));
      AddMemberInitialiserConstructor(ctor);
   }
}

void KVClassFactory::AddDestructor(const TString& access)
{
   // Add default destructor to class

   if (!GetDestructor()) {
      KVClassDestructor* d = new KVClassDestructor;
      d->SetClassName(fClassName);
      d->SetAccess(access);
      fMethods.Add(d);
   }
}

KVClassFactory::KVClassMember::KVClassMember(const Char_t* name, const Char_t* type, const Char_t* comment, const Char_t* access)
   : KVBase(Form("f%s", name), type), fComment(comment), fRealName(name)
{
   // New class member variable
   SetAccess(access);
}

/** \example KVClassFactory_examples.C
# Example of class source code generation using KVClassFactory

KVClassFactory can be used to generate skeleton class source files

To execute this function, do

    $ kaliveda
    kaliveda[0] .L KVClassFactory_examples.C+
    kaliveda[1] make_examples()
*/
