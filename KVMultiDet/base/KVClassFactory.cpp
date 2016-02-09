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
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Factory class for generating skeleton files for new classes.
//It can generate basic '.h' and '.cpp' files for
//      - base classes (base_class="")
//      - inherited classes (base_class!="")
//      - classes based on template files
//
//There are two ways to use KVClassFactory in order to generate code:
//
//1. Using the static (stand-alone) method
//
//      KVClassFactory::MakeClass(classname, classdesc, base_class, withTemplate, templateFile);
//
//2. Create and configure a KVClassFactory object and then call the GenerateCode() method:
//
//    KVClassFactory fact(classname, classdesc, base_class, withTemplate, templateFile);
//    fact.GenerateCode();
//
//The second method is more flexible and allows to add methods to classes before
//code generation, even if the class is created from a predefined template.
//
//For example, let us suppose that we want to add the following method to our class:
//
//   declaration:
//      virtual const Char_t* GetName(Option_t* = "") const;
//
//   implementation:
//      const Char_t* MyNewClass::GetName(Option_t* opt) const
//      {
//            //A new method
//            if( strcmp(opt,"") ) cout << "Option : " << opt << endl;
//            else cout << fName.Data() << endl;
//            return fName.Data();
//      }
//
//This can be done as follows:
//
//    fact.AddMethod("GetName", "const Char_t*", kTRUE, kTRUE);
//    fact.AddMethodArgument("GetName", "Option_t*", "opt", "\"\"");
//    KVString body;
//    body += "      //A new method\n";
//    body += "      if( strcmp(opt,\"\") ) cout << \"Option : \" << opt << endl;\n";
//    body += "      else cout << fName.Data() << endl;\n";
//    body += "      return fName.Data();";
//    fact.AddMethodBody("GetName", body);
//
//The addition of a new method may mean that it is necessary to add an '#include'
//directive to either the header or the implementation file of the new class. For example,
//in this case, the use of 'cout', 'endl' etc. may require to add an '#include "Riostream.h"'
//to the '.cpp' file of the new class. This can be done as follows:
//
//    fact.AddImplIncludeFile("Riostream.h");
//
//For another example of this kind of approach, see the method KVParticleCondition::Optimize.

KVClassFactory::KVClassFactory()
{
   fHasBaseClass = kFALSE;
   fBaseClassTObject = kFALSE;
   fBaseClass = NULL;
}

KVClassFactory::KVClassFactory(const Char_t* classname,
                               const Char_t* classdesc,
                               const Char_t* base_class,
                               Bool_t withTemplate,
                               const Char_t* templateFile)
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
   //Write date of creation and name of USER who called ClassFactory

   file << "//Created by KVClassFactory on " << fNow.AsString() << endl;
   file << "//Author: " << fAuthor.Data() << "\n" << endl;
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
      } else
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
   // This will include a default (public) constructor & destructor and
   // any methods added using AddMethod, sorted according to
   // their access type.

   file << "class " << fClassName.Data();
   if (fHasBaseClass) {
      file << " : ";
      if (WithMultipleBaseClasses()) {
         fBaseClassName.Begin(",");
         file << "public " << fBaseClassName.Next(kTRUE);
         while (!fBaseClassName.End()) file << ", public " << fBaseClassName.Next(kTRUE);
      } else
         file << "public " << fBaseClassName.Data();
   }
   file << "\n{" << endl;

   // protected members
   KVList* prem = (KVList*)fMembers.GetSubListWithMethod("protected", "GetAccess");
   if (prem->GetEntries()) {
      file << "\n   protected:" << endl;
      KVString line;
      TIter next(prem);
      KVClassMember* meth;
      while ((meth = (KVClassMember*)next())) {
         meth->WriteDeclaration(line);
         file << "   " << line.Data() << endl;
      }
   }
   delete prem;

   // private methods
   KVList* priv = (KVList*)fMethods.GetSubListWithMethod("private", "GetAccess");
   if (priv->GetEntries()) {
      file << "   private:" << endl;
      KVString line;
      TIter next(priv);
      KVClassMethod* meth;
      while ((meth = (KVClassMethod*)next())) {
         meth->WriteDeclaration(line);
         file << "   " << line.Data() << endl;
      }
   }
   delete priv;

   // protected methods
   KVList* prot = (KVList*)fMethods.GetSubListWithMethod("protected", "GetAccess");
   if (prot->GetEntries()) {
      file << "\n   protected:" << endl;
      KVString line;
      TIter next(prot);
      KVClassMethod* meth;
      while ((meth = (KVClassMethod*)next())) {
         meth->WriteDeclaration(line);
         file << "   " << line.Data() << endl;
      }
   }
   delete prot;

   //public methods
   file << "\n   public:" << endl;
   //default ctor
   file << "   " << fClassName.Data() << "();" << endl;
   //any other ctors ?
   KVList* ctor = (KVList*)fMethods.GetSubListWithMethod("1", "IsConstructor");
   if (ctor->GetEntries()) {
      KVString line;
      TIter next(ctor);
      KVClassMethod* meth;
      while ((meth = (KVClassMethod*)next())) {
         meth->WriteDeclaration(line);
         file << "   " << line.Data() << endl;
      }
   }
   delete ctor;
   // default dtor
   file << "   virtual ~" << fClassName.Data() << "();" << endl;

   // public methods
   KVList* pub = (KVList*)fMethods.GetSubListWithMethod("public", "GetAccess");
   if (pub->GetEntries()) {
      KVString line;
      TIter next(pub);
      KVClassMethod* meth;
      while ((meth = (KVClassMethod*)next())) {
         if (!meth->IsConstructor()) {
            meth->WriteDeclaration(line);
            file << "   " << line.Data() << endl;
         }
      }
   }
   delete pub;

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

   file_cpp.open(GetImpFileName());

   WriteWhoWhen(file_cpp);

   file_cpp << "#include \"" << fClassName.Data() << ".h\"" << endl;
   if (fImpInc.GetSize()) {
      TIter next(&fImpInc);
      TObjString* str;
      while ((str = (TObjString*)next())) {
         file_cpp << "#include \"" << str->String().Data() << "\"" << endl;
      }
   }
   file_cpp << endl << "ClassImp(" << fClassName.Data() << ")\n" << endl;
   file_cpp <<
            "////////////////////////////////////////////////////////////////////////////////"
            << endl;
   file_cpp << "// BEGIN_HTML <!--" << endl;
   file_cpp << "/* -->" << endl;
   file_cpp << "<h2>" << fClassName.Data() << "</h2>" << endl;
   file_cpp << "<h4>" << fClassDesc.Data() << "</h4>" << endl;
   file_cpp << "<!-- */" << endl;
   file_cpp << "// --> END_HTML" << endl;
   file_cpp <<
            "////////////////////////////////////////////////////////////////////////////////\n"
            << endl;
   file_cpp << fClassName.Data() << "::" << fClassName.
            Data() << "()" << endl;
   file_cpp << "{\n   // Default constructor\n}\n" << endl;
   // any other ctors ?
   KVList* ctor = (KVList*)fMethods.GetSubListWithMethod("1", "IsConstructor");
   if (ctor->GetEntries()) {
      KVString line;
      TIter next(ctor);
      KVClassMethod* meth;
      while ((meth = (KVClassMethod*)next())) {
         meth->WriteImplementation(line);
         line.Prepend("//________________________________________________________________\n\n");
         file_cpp << line.Data() << endl;
      }
   }
   delete ctor;

   file_cpp << fClassName.Data() << "::~" << fClassName.
            Data() << "()" << endl;
   file_cpp << "{\n   // Destructor\n}\n" << endl;

   //write implementations of added methods
   if (fMethods.GetSize()) {
      KVString line;
      TIter next(&fMethods);
      KVClassMethod* meth;
      while ((meth = (KVClassMethod*)next())) {
         if (!meth->IsConstructor()) {
            meth->WriteImplementation(line);
            line.Prepend("//________________________________________________________________\n\n");
            file_cpp << line.Data() << endl;
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

   file_h.open(GetHeaderFileName());

   WriteWhoWhen(file_h);
   WritePreProc(file_h);
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
   //Give a name for the class and a short description, used for HTML doc.
   //The optional string 'base_class' gives the name(s) of the parent class(es)*, in case of inheritance;
   //if not given, the new class will be a base class.
   //    *(in case of several base classes, give a comma-separated list)
   //
   //if withTemplate=kTRUE, we use a template for the class structure, defined in a '.h' and a '.cpp' file,
   //as follows:
   //
   //      if templateFile="" (default), we expect base_class!="", and template files with names base_classTemplate.h and base_classTemplate.cpp
   //      must be present in either $KVROOT/KVFiles, $HOME or $PWD directories.
   //      the dummy classname "base_classTemplate" will be replaced everywhere by 'classname'
   //
   //      if templateFile="/absolute/path/classTemplate" we use classTemplate.h & classTemplate.cpp in the given directory.
   //      the dummy classname "classTemplate" will be replaced everywhere by 'classname'
   //
   //      if templateFile="classTemplate" we look for classTemplate.h & classTemplate.cpp in $KVROOT/KVFiles, $HOME or $PWD directories.
   //      the dummy classname "classTemplate" will be replaced everywhere by 'classname'
   //
   //Example of use:
   //      KVClassFactory::MakeClass("MyClass", "A brand new class", "TObject")
   //
   //will generate the following MyClass.h and MyClass.cpp files:
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

   KVClassFactory cf(classname, classdesc, base_class, withTemplate, templateFile);
   if (!cf.IsZombie()) cf.GenerateCode();
}

void KVClassFactory::GenerateCode()
{
   //Generate header and implementation file for currently-defined class

   if (IsZombie()) {
      Warning("GenerateCode", "Object is zombie. No code will be generated.");
      return;
   }

   if (fWithTemplate) {
      WriteClassWithTemplateHeader();
      WriteClassWithTemplateImp();
   } else {
      if (fBaseClassTObject) {
         // check for base class which inherits from TObject
         // if so, we add a skeleton Copy(const TObject&) method
         // and use it in the copy ctor
         AddTObjectCopyMethod();
         //AddCopyConstructor(kTRUE);
      }
//       else
//          AddCopyConstructor(kFALSE);
      AddAllBaseConstructors();
      WriteClassHeader();
      WriteClassImp();
   }
}

KVClassMember* KVClassFactory::AddMember(const Char_t* name, const Char_t* type, const Char_t* comment, const Char_t* access)
{
   // Add a member variable to the class, with name 'f[name]' according to ROOT convention.
   // The access type is by default "protected", in accordance with OO-encapsulation.

   KVClassMember* m = new KVClassMember(name, type, comment, access);
   fMembers.Add(m);
   return m;
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
      } else if (file(pos) == '\n') {
         // reached newline without seeing comment
         // if there is non-whitespace stuff on line, this is OK
         if (nonWS) return beg_cur_line;
         pos++;
         beg_cur_line = pos;
      } else if (file(pos) == '/') {

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
         } else if (file(pos + 1) == '/') {
            // beginning of 1-line comment
            // was there non-whitespace stuff before this ?
            if (nonWS) return beg_cur_line;
            Ssiz_t len = file.Index('\n', pos + 2); // find end of line
            if (len > 0) {
               pos = len + 1;
               beg_cur_line = pos;
            } else {
               // end of comment not found in rest of file
               // return -1
               pos = -1;
               break;
            }
         }
      } else {
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

   file_h.open(GetHeaderFileName());

   WriteWhoWhen(file_h);
   WritePreProc(file_h);

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
      if (fMethods.GetSize()) {
         KVString line;
         TIter next(&fMethods);
         KVClassMethod* meth;
         while ((meth = (KVClassMethod*)next())) {
            meth->WriteDeclaration(line);
            part_add += line;
            part_add += "\n";
         }
      }

      //stick all parts together
      headFile = part1 + part_add + part2 + part3;
   }

   file_h << headFile.ReplaceAll(fTemplateClassName.Data(),
                                 fClassName.Data());
   file_h << "\n\n#endif" << endl;      //don't forget to close the preprocessor #if !!!
   file_h.close();

   cout << "<KVClassFactory::WriteClassWithTemplateHeader> : File " << GetHeaderFileName() << " generated." << endl;

}

//___________________________________________________
void KVClassFactory::WriteClassWithTemplateImp()
{
   // Writes the implementation file for the class

   ofstream file_cpp;

   file_cpp.open(GetImpFileName());

   WriteWhoWhen(file_cpp);

   file_cpp << "#include \"" << fClassName.Data() << ".h\"" << endl;
   if (fImpInc.GetSize()) {
      TIter next(&fImpInc);
      TObjString* str;
      while ((str = (TObjString*)next())) {
         file_cpp << "#include \"" << str->String().Data() << "\"" << endl;
      }
   }
   file_cpp << endl << "ClassImp(" << fClassName.Data() << ")\n" << endl;
   file_cpp <<
            "////////////////////////////////////////////////////////////////////////////////"
            << endl;
   file_cpp << "// BEGIN_HTML <!--" << endl;
   file_cpp << "/* -->" << endl;
   file_cpp << "<h2>" << fClassName.Data() << "</h2>" << endl;
   file_cpp << "<h4>" << fClassDesc.Data() << "</h4>" << endl;
   file_cpp << "<!-- */" << endl;
   file_cpp << "// --> END_HTML" << endl;
   file_cpp <<
            "////////////////////////////////////////////////////////////////////////////////\n"
            << endl;

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
         line.Prepend("\n//________________________________________________________________\n");
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
   } else {
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

KVClassMethod* KVClassFactory::AddMethod(const Char_t* name, const Char_t* return_type, const Char_t* access,
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

KVClassMethod* KVClassFactory::AddConstructor(const Char_t* argument_type,
      const Char_t* argument_name, const Char_t* default_value, const Char_t* access)
{
   // Add a constructor with arguments to the class (by default, a default constructor is always defined).
   //
   // Optional argument 'access' determines access type (public, protected or private)    [default: "public"]
   //
   // If more than one argument is needed, user should keep the returned pointer
   // to the new KVClassMethod object and use KVClassMethod::AddArgument() in order to add further arguments.
   //
   // In order to define the implementation of the ctor method, user should keep the returned pointer
   // to the new KVClassMethod object and use KVClassMethod::SetMethodBody(KVString&).

   KVClassMethod* meth = new KVClassMethod;
   fMethods.Add(meth);
   // use type of first argument as name of ctor method
   meth->SetName(argument_type);
   meth->SetClassName(fClassName);
   if (fHasBaseClass) meth->SetBaseClass(fBaseClassName);
   meth->SetConstructor();
   meth->SetAccess(access);
   meth->AddArgument(argument_type, argument_name, default_value);
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

void KVClassFactory::AddMethodBody(const Char_t* method_name, KVString& body)
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
   // For each named parameter in the list, we add protected member variables and
   // public Get/Set methods with the name and type of the parameter.
   // Example: given a list containing NAME="some string" NUMBER=3 [integer]
   // PI=3.141592 [double], we generate the methods
   //   void SetNAME(const TString&);
   //   const TString& GetNAME() const;
   //   void SetNUMBER(Int_t);
   //   Int_t GetNUMBER() const;
   //   void SetPI(Double_t);
   //   Double_t GetPI() const;

   int npars = nvl.GetNpar();
   for (int i = 0; i < npars; i++) {
      KVNamedParameter* par = nvl.GetParameter(i);
      if (par->IsString()) {
         KVClassMember* v = AddMember(par->GetName(), "TString", "member automatically generated by KVClassFactory::AddGetSetMethods");
         KVClassMethod* m = AddMethod(Form("Set%s", par->GetName()), "void");
         m->AddArgument("const TString&");
         m->SetMethodBody(
            Form("    // Method automatically generated by KVClassFactory::AddGetSetMethods\n"
                 "\n"
                 "    %s=arg1;", v->GetName())
         );
         m = AddMethod(Form("Get%s", par->GetName()), "const TString&", "public", kFALSE, kTRUE);
         m->SetMethodBody(
            Form("    // Method automatically generated by KVClassFactory::AddGetSetMethods\n"
                 "\n"
                 "    return %s;", v->GetName())
         );
         // make sure #include "TString.h" appears in header file
         if (! fHeadInc.FindObject("TString.h")) AddHeaderIncludeFile("TString.h");
      } else if (par->IsInt()) {
         KVClassMember* v = AddMember(par->GetName(), "Int_t", "member automatically generated by KVClassFactory::AddGetSetMethods");
         KVClassMethod* m = AddMethod(Form("Set%s", par->GetName()), "void");
         m->AddArgument("Int_t");
         m->SetMethodBody(
            Form("    // Method automatically generated by KVClassFactory::AddGetSetMethods\n"
                 "\n"
                 "    %s=arg1;", v->GetName())
         );
         m = AddMethod(Form("Get%s", par->GetName()), "Int_t", "public", kFALSE, kTRUE);
         m->SetMethodBody(
            Form("    // Method automatically generated by KVClassFactory::AddGetSetMethods\n"
                 "\n"
                 "    return %s;", v->GetName())
         );
      } else if (par->IsDouble()) {
         KVClassMember* v = AddMember(par->GetName(), "Double_t", "member automatically generated by KVClassFactory::AddGetSetMethods");
         KVClassMethod* m = AddMethod(Form("Set%s", par->GetName()), "void");
         m->AddArgument("Double_t");
         m->SetMethodBody(
            Form("    // Method automatically generated by KVClassFactory::AddGetSetMethods\n"
                 "\n"
                 "    %s=arg1;", v->GetName())
         );
         m = AddMethod(Form("Get%s", par->GetName()), "Double_t", "public", kFALSE, kTRUE);
         m->SetMethodBody(
            Form("    // Method automatically generated by KVClassFactory::AddGetSetMethods\n"
                 "\n"
                 "    return %s;", v->GetName())
         );
      }
   }
}

//__________________________________________________________________________________

void KVClassFactory::AddTObjectCopyMethod()
{
   // Adds skeleton standard ROOT Copy method
   AddMethod("Copy", "void", "public", kFALSE, kTRUE);
   AddMethodArgument("Copy", "TObject&", "obj");
   KVString body("   // This method copies the current state of 'this' object into 'obj'\n");
   body += "   // You should add here any member variables, for example:\n";
   body += "   //    (supposing a member variable ";
   body += fClassName;
   body += "::fToto)\n";
   body += "   //    CastedObj.fToto = fToto;\n";
   body += "   // or\n";
   body += "   //    CastedObj.SetToto( GetToto() );\n\n   ";
   // call Copy method for base class
   body += fBaseClassName;
   body += "::Copy(obj);\n   //";
   body += fClassName;
   body += "& CastedObj = (";
   body += fClassName;
   body += "&)obj;";
   AddMethodBody("Copy", body);
}

void KVClassFactory::AddCopyConstructor(Bool_t withTObjectCopy)
{
   // Adds copy constructor
   // If class inherits from TObject, this just calls the Copy method

   KVClassMethod* ctor = AddConstructor(Form("const %s&", fClassName.Data()), "obj");
   ctor->SetCopyCtor();
   KVString body = "   // Copy constructor\n";
   body += "   // This ctor is used to make a copy of an existing object (for example\n";
   body += "   // when a method returns an object), and it is always a good idea to\n";
   body += "   // implement it.\n";
   body += "   // If your class allocates memory in its constructor(s) then it is ESSENTIAL :-)\n";
   if (withTObjectCopy) {
      body += "\n   obj.Copy(*this);";
   }
   ctor->SetMethodBody(body);
}

//___________________________________________________________________________________


ClassImp(KVClassMethod)
ClassImp(KVClassMember)
/////////////////////////////////////////////////////////////////////////////////////
//Helper classes for KVClassFactory

void KVClassMethod::WriteDeclaration(KVString& decl)
{
   //Write declaration in the KVString object

   decl = GetAccess();
   decl += ":\n   ";
   if (fVirtual) decl += "virtual ";
   if (!IsConstructor()) {
      decl += GetReturnType();
      decl += " ";
      decl += GetName();
   } else decl += GetClassName();
   decl += "(";
   for (int i = 1; i <= fNargs; i++) {
      decl += fFields.GetStringValue(Form("Arg_%d", i));
      if (fFields.HasParameter(Form("Arg_%d_name", i))) {
         decl += " ";
         decl += fFields.GetStringValue(Form("Arg_%d_name", i));
      } else
         decl += Form(" arg%d", i);
      if (fFields.HasParameter(Form("Arg_%d_default", i))) {
         decl += " = ";
         decl += fFields.GetStringValue(Form("Arg_%d_default", i));
      }
      if (i < fNargs) decl += ", ";
   }
   decl += ")";
   if (fConst) decl += " const";
   decl += ";";
}

void KVClassMember::WriteDeclaration(KVString& decl)
{
   //Write declaration in the KVString object

   decl = GetType();
   decl += "\t\t";
   decl += GetName();

   decl += ";//";
   decl += GetComment();
}

void KVClassMethod::WriteImplementation(KVString& decl)
{
   //Write skeleton implementation in the KVString object
   //All constructors call the default ctor of the base class (if defined)
   decl = GetReturnType();
   if (!IsConstructor()) {
      decl += " ";
   }
   decl += GetClassName();
   decl += "::";
   if (!IsConstructor())
      decl += GetName();
   else
      decl += GetClassName();
   decl += "(";
   for (int i = 1; i <= fNargs; i++) {
      decl += fFields.GetStringValue(Form("Arg_%d", i));
      if (fFields.HasParameter(Form("Arg_%d_name", i))) {
         decl += " ";
         decl += fFields.GetStringValue(Form("Arg_%d_name", i));
      } else
         decl += Form(" arg%d", i);
      if (i < fNargs) decl += ", ";
   }
   decl += ")";
   if (fConst) decl += " const";
   if (IsConstructor() && fFields.HasParameter("BaseClass")) {
      decl += " : ";
      decl += fFields.GetStringValue("BaseClass");
      decl += "(";
      if (!IsCopyCtor() && fNargs) {
         for (int i = 1; i <= fNargs; i++) {
            if (fFields.HasParameter(Form("Arg_%d_name", i))) {
               decl += fFields.GetStringValue(Form("Arg_%d_name", i));
            } else
               decl += Form("arg%d", i);
            if (i < fNargs) decl += ", ";
         }
      }
      decl += ")";
   }
   decl += "\n{\n";
   if (!strcmp(GetAccess(), "private")) decl += "   // PRIVATE method\n";
   else if (!strcmp(GetAccess(), "protected")) decl += "   // PROTECTED method\n";
   if (fFields.HasParameter("Body")) {
      //write body of method
      decl += fFields.GetStringValue("Body");
   } else {
      decl += "   // Write your code here";
      if (fFields.HasParameter("ReturnType") && strcmp(fFields.GetStringValue("ReturnType"), "void")) {
         decl += "\n   return (";
         decl += GetReturnType();
         decl += ")0;";
      }
   }
   decl += "\n}\n";
}

//__________________________________________________________________________________

KVClassMember::KVClassMember(const KVClassMember& obj) : KVBase()
{
   //ctor par copie
   obj.Copy(*this);
}

//__________________________________________________________________________________

KVClassMethod::KVClassMethod(const KVClassMethod& obj) : KVClassMember()
{
   //ctor par copie
   obj.Copy(*this);
}

//__________________________________________________________________________________

void KVClassMember::Copy(TObject& obj) const
{
   //copy this to obj
   KVBase::Copy(obj);
   ((KVClassMember&)obj).SetComment(GetComment());
}

//__________________________________________________________________________________

void KVClassMethod::Copy(TObject& obj) const
{
   //copy this to obj
   KVClassMember::Copy(obj);
   ((KVClassMethod&)obj).SetVirtual(IsVirtual());
   ((KVClassMethod&)obj).SetVirtual(IsVirtual());
   ((KVClassMethod&)obj).SetConstructor(IsConstructor());
   fFields.Copy(((KVClassMethod&)obj).fFields);
   ((KVClassMethod&)obj).SetNargs(fNargs);
}

//__________________________________________________________________________________
void KVClassMethod::Print(Option_t*) const
{
   // print the KVClass method
   cout << "KVClassMethod object -----> " << GetName() << endl;
   fFields.Print();
   cout << "This method is " << GetAccess() << endl;
   if (fConst) cout << "This method is CONST" << endl;
   if (fVirtual) cout << "This method is VIRTUAL" << endl;
}

//__________________________________________________________________________________
void KVClassMember::Print(Option_t*) const
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
   // By default, all constructors are 'public'.

   if (!fBaseClass) return;

   KVHashList clist;
   clist.AddAll(fBaseClass->GetListOfMethods());
   KVSeqCollection* constructors = clist.GetSubListWithName(fBaseClassName);
   TIter next_ctor(constructors);
   TMethod* method;
   while ((method = (TMethod*)next_ctor())) {
      if (!method->GetNargs()) continue; // ignore default ctor
      TList* args = method->GetListOfMethodArgs();
      TMethodArg* arg = (TMethodArg*)args->First();
      TString typenam = arg->GetFullTypeName();
      if (typenam.Contains(fBaseClassName)) continue; // ignore copy ctor
      KVClassMethod* ctor;
      if (arg->GetDefault()) ctor = AddConstructor(typenam, arg->GetName(), arg->GetDefault());
      else ctor = AddConstructor(typenam, arg->GetName());
      for (int i = 1; i < method->GetNargs(); i++) {
         arg = (TMethodArg*)args->At(i);
         if (arg->GetDefault()) ctor->AddArgument(arg->GetFullTypeName(), arg->GetName(), arg->GetDefault());
         else ctor->AddArgument(arg->GetFullTypeName(), arg->GetName());
      }
   }

   delete constructors;
}


KVClassMember::KVClassMember(const Char_t* name, const Char_t* type, const Char_t* comment, const Char_t* access)
   : KVBase(Form("f%s", name), type)
{
   // New class member variable
   SetComment(comment);
   SetAccess(access);
}
