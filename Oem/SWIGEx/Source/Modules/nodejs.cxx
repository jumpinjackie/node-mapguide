#include "swigmod.h"
#include <vector>
#include <map>
#include <set>
#include <sstream>

#define EXPORTS_VAR "exports"
#define CLASS_EXPORT_VAR "constructor_tpl"
#define CLASS_PROXY_CTOR_NAME "New"
#define BASE_CLASS_FUNC_TEMPLATE_VAR "baseCls"
#define WRAPPED_PTR_VAR "wrappedPtr"
#define FUNC_RET_VAR "retVal"
#define ARGS_VAR "args"

extern std::map<std::string, int> clsIds;

class Indenter
{
private:
    int& m_level;

public:
    Indenter(int& level) : m_level(level)
    { 
        m_level++;
    }
    ~Indenter() { m_level--; }
};

class FragmentWriter
{
private:
    std::string& m_fragment;
    int& m_indent;

public:
    FragmentWriter(int& indentLevel, std::string& fragment) : m_indent(indentLevel), m_fragment(fragment)
    {
    }

    int& indent() { return m_indent; }

    void writeTabs() 
    {
        if (m_indent > 0)
        {
            for (int i = 0; i < m_indent; i++)
            {
                m_fragment += "\t";
            }
        }
    }

    void writeToFragment(String* str, bool bDelete = true)
    {
        writeTabs();
        m_fragment += Char(str);
        if (bDelete)
            Delete(str);
    }

    void writeToFragment(const std::string& str)
    {
        writeTabs();
        m_fragment += str;
    }

    void writeToFragment(const char* str)
    {
        writeTabs();
        m_fragment += str;
    }
};

class ExceptionCodeWriter
{
private:
    FragmentWriter& m_frag;
    std::string m_try_macro;
    std::string m_catch_macro;
    std::string m_methodName;
    bool m_bIncludeRawPointerCleanup;

public:
    ExceptionCodeWriter(FragmentWriter& frag, 
                        std::string methodName, 
                        std::string tryMacro = "MG_TRY", 
                        std::string catchMacro = "MG_CATCH",
                        bool bIncludeRawPointerCleanup = false) 
        : m_frag(frag),
          m_try_macro(tryMacro),
          m_catch_macro(catchMacro),
          m_methodName(methodName),
          m_bIncludeRawPointerCleanup(bIncludeRawPointerCleanup)
    {
        m_frag.writeToFragment(NewStringf("%s()\n", m_try_macro.c_str()));
    }

    ~ExceptionCodeWriter()
    {
        m_frag.writeToFragment(NewStringf("%s(L\"%s\")\n", m_catch_macro.c_str(), m_methodName.c_str()));
        m_frag.writeToFragment("if (mgException != NULL)\n");
        m_frag.writeToFragment("{\n");
        {
            Indenter ind2(m_frag.indent());
            m_frag.writeToFragment("//NOTE: Unlike previous iterations of the MapGuide API, MgException::GetDetails() finally gives us *everything*\n");
            m_frag.writeToFragment("STRING details = mgException->GetDetails();\n");
            m_frag.writeToFragment("std::string mbDetails = MgUtil::WideCharToMultiByte(details);\n");
            m_frag.writeToFragment("v8::Local<v8::Value> v8ex = v8::Exception::Error(v8::String::New(mbDetails.c_str()));\n");
            m_frag.writeToFragment("//Release our MgException before throwing the v8 one\n");
            m_frag.writeToFragment("mgException = NULL;\n");
            if (m_bIncludeRawPointerCleanup)
                m_frag.writeToFragment(NewStringf("if (%s) { ((MgDisposable*)%s)->Release(); %s = NULL; }\n", WRAPPED_PTR_VAR, WRAPPED_PTR_VAR, WRAPPED_PTR_VAR));
            m_frag.writeToFragment("return v8::ThrowException(v8ex);\n");
        }
        m_frag.writeToFragment("}\n");
    }
};

class ArgInfo
{
public:
    ArgInfo(std::string sName, std::string sType, int iOffset, bool bMgPointer)
    {
        name = sName;
        type = sType;
        isMgPointer = bMgPointer;
        offset = iOffset;
    }

    std::string name;
    std::string type;
    bool isMgPointer;
    int offset;
};

class FunctionCallSetup
{
private:
    std::string m_funcArgsStr;
    std::vector<ArgInfo> m_args;
public:
    FunctionCallSetup()
    {
    }

    void setFunctionArgumentString(std::string str) { m_funcArgsStr = str; }
    const char* getFunctionArgumentString() const { return m_funcArgsStr.c_str(); }

    const std::vector<ArgInfo>& getArgs() const { return m_args; }

    void addArg(std::string sName, std::string sType, int argOffset, bool bMgPointer)
    {
        ArgInfo ai(sName, sType, argOffset, bMgPointer);
        m_args.push_back(ai);
    }
};

typedef std::map<std::string, std::string> StringMap;
typedef std::vector<std::string> StringList;
typedef std::set<std::string> StringSet;
typedef std::map<std::string, StringList> StringListMap;
typedef std::map<std::string, StringSet> StringSetMap;

class NODEJS : public Language
{
private:
    String* m_namespace;
    String* m_currentClassName;
    File* m_fOutputFile;
    StringMap m_classDecls;
    StringList m_classMethodDecls;
    StringList m_classExportCalls;
    StringList m_classInheritCalls;
    StringList m_classMemberExportCalls;
    StringList m_classSetupCalls;
    StringMap m_ctorSetupCode;
    StringSetMap m_classMethodOverloads;
    StringMap m_classMethodOverloadResolution;
    std::string m_currentFragment;
    std::string m_currentExportFragment;
    std::string m_currentInheritFragment;
    std::string m_currentMethodFragment;
    std::string m_currentClassSetupFragment;
    int m_indentationLevels;
    bool m_bInsertDebuggingCode;

public:
    NODEJS() : 
        m_namespace(NULL),
        m_currentClassName(NULL),
        m_fOutputFile(NULL),
        m_indentationLevels(0),
        m_bInsertDebuggingCode(true)
    { }

    virtual void main(int argc, char* argv[])
    {
        printf("NodeJS module\n");
        for (int i = 1; i < argc; i++)
        {
            if (argv[i])
            {
                if (strcmp(argv[i], "-namespace") == 0)
                {
                    if (argv[i+1])
                    {
                        m_namespace = NewString("");
                        Printf(m_namespace, argv[i+1]);
                        Swig_mark_arg(i);
                        Swig_mark_arg(i+1);
                        i++;
                    }
                    else
                    {
                        Swig_arg_error();
                    }
                }
                else if (strcmp(argv[i], "") == 0)
                {
                    Swig_mark_arg(i);
                }
            }
        }
        
        Preprocessor_define("SWIGNODEJS 1", 0);
        SWIG_typemap_lang("nodejs");
    }

    virtual int top(Node* n)
    {
        String *outfile = Getattr(n,"outfile");
        Printf(stdout, "Generating code to %s\n", outfile);
        
        m_fOutputFile = NewFile(outfile, "w");
        if (!m_fOutputFile)
        {
            Printf(stderr, "Unable to open %s\n", outfile);
            SWIG_exit(EXIT_FAILURE);
        }

        String* modName = NewString(Getattr(n, "name"));
        Swig_banner(m_fOutputFile);

        writeToFile("#include <node.h>\n\n\n");
        writeToFile("#include <node_buffer.h>\n\n\n");

        writeToFile("//TODO: Should learn how to process SWIG %module directive instead of hard-coding headers\n");
        writeToFile("#include <map>\n");
        writeToFile("#include \"MapGuideCommon.h\"\n");
        writeToFile("#include \"WebApp.h\"\n");
        writeToFile("#include \"WebSupport.h\"\n");
        writeToFile("#include \"HttpHandler.h\"\n\n\n");

        // Due to C++ being sensitive to order of types declared (we are outputting this all into one big .cpp file)
        // We have to output the generated code in chunks, as shown below
        //
        //

        Language::top(n);

        writeToFile("//=====================================\n");
        writeToFile("//Polymorphism support\n");
        writeToFile("//=====================================\n");
        writeToFile("class JsProxyFactory\n");
        writeToFile("{\n");
        writeToFile("public:\n");
        {
            Indenter ind(m_indentationLevels);
            writeToFile("static v8::Handle<v8::Value> CreateJsProxy(MgObject* obj);\n");
        }
        writeToFile("};\n\n");

        writeToFile("//=====================================\n");
        writeToFile("//Proxy class definitions\n");
        writeToFile("//=====================================\n");
        for (StringMap::iterator it = m_classDecls.begin(); it != m_classDecls.end(); it++)
        {
            std::string call = it->second;
            writeToFile(call);
            writeToFile("\n");
        }

        // Add main polymorphism support code
        writeToFile("//=====================================\n");
        writeToFile("//Polymorphism support implementation\n");
        writeToFile("//=====================================\n");
        writeToFile("v8::Handle<v8::Value> JsProxyFactory::CreateJsProxy(MgObject* obj)\n");
        writeToFile("{\n");
        {
            Indenter ind(m_indentationLevels);
            writeToFile("CHECKARGUMENTNULL(obj, L\"JsProxyFactory::CreateJsProxy\");\n");

            writeToFile("INT32 clsId = obj->GetClassId();\n");
            writeToFile("switch(clsId)\n");
            writeToFile("{\n");
            {
                Indenter ind2(m_indentationLevels);
                for(std::map<std::string, int>::const_iterator it = clsIds.begin(); it != clsIds.end(); it++)
                {
                    std::string clsName = it->first;
                    //Skip exception classes
                    if (clsName.find("Exception") != std::string::npos)
                        continue;

                    int clsId = it->second;

                    std::string proxyClsName;
                    setProxyClassName(clsName.c_str(), proxyClsName);
                    writeToFile(NewStringf("case %d: return %s::CreateJsProxy(obj);\n", clsId, proxyClsName.c_str()));
                }
                writeToFile("default: return v8::ThrowException(v8::Exception::Error(v8::String::New(\"Could not find JavaScript proxy factory for unmanaged pointer\")));\n");
            }
            writeToFile("}\n");
        }
        writeToFile("}\n");

        writeToFile("//=====================================\n");
        writeToFile("//v8 persistent Proxy class constructors\n");
        writeToFile("//=====================================\n");
        for (StringMap::iterator it = m_classDecls.begin(); it != m_classDecls.end(); it++)
        {
            std::string clsName = it->first;
            std::string proxyClassName;
            setProxyClassName(clsName.c_str(), proxyClassName);

            writeToFile(NewStringf("v8::Persistent<v8::FunctionTemplate> %s::constructor_tpl;\n", proxyClassName.c_str()));
            writeToFile(NewStringf("v8::Persistent<v8::Function> %s::constructor;\n", proxyClassName.c_str()));
        }

        writeToFile("//=====================================\n");
        writeToFile("//Proxy class method definitions\n");
        writeToFile("//=====================================\n");
        for (StringList::iterator it = m_classMethodDecls.begin(); it != m_classMethodDecls.end(); it++)
        {
            std::string call = (*it);
            writeToFile(call);
            writeToFile("\n");
        }

        writeToFile("//=====================================\n");
        writeToFile("//Proxy class method overloaded methods\n");
        writeToFile("//=====================================\n");
        for (StringSetMap::iterator it = m_classMethodOverloads.begin(); it != m_classMethodOverloads.end(); it++)
        {
            std::string clsName = it->first;
            StringSet lst = it->second;
            for (StringSet::iterator it2 = lst.begin(); it2 != lst.end(); it2++)
            {
                std::string ovMethodName = (*it2);
                writeToFile(NewStringf("//Overloaded method %s::%s\n", clsName.c_str(), ovMethodName.c_str()));
                writeToFile(NewStringf("static v8::Handle<v8::Value> %s(const v8::Arguments& %s)\n", ovMethodName.c_str(), ARGS_VAR));
                writeToFile("{\n");
                {
                    Indenter ind(m_indentationLevels);
                    StringMap::iterator it3 = m_classMethodOverloadResolution.find(ovMethodName);
                    if (it3 != m_classMethodOverloadResolution.end())
                    {
                        writeToFile(it3->second);
                        writeToFile("return v8::ThrowException(v8::Exception::Error(v8::String::New(\"Could not resolve appropriate overloaded method based on the arguments given\")));\n");
                    }
                    else
                    {
                        writeToFile("//FIXME: This is probably a bug. An registered overloaded method should have matching resolution code\n");
                    }
                }
                writeToFile("}\n");
            }
        }

        writeToFile("//=====================================\n");
        writeToFile("//Proxy class constructor definitions\n");
        writeToFile("//=====================================\n");
        for (StringMap::iterator it = m_classDecls.begin(); it != m_classDecls.end(); it++)
        {
            std::string clsName = it->first;
            std::string proxyClsName;
            setProxyClassName(clsName.c_str(), proxyClsName);
            writeToFile(NewStringf("v8::Handle<v8::Value> %s::%s(const v8::Arguments& %s)\n", proxyClsName.c_str(), CLASS_PROXY_CTOR_NAME, ARGS_VAR));
            writeToFile("{\n");
            if (m_ctorSetupCode.find(clsName) != m_ctorSetupCode.end())
            {
                std::string code;
                {
                    Indenter ind(m_indentationLevels);
                    FragmentWriter codeWriter(m_indentationLevels, code);
                    {
                        std::string mbMethodName;
                        mbMethodName += proxyClsName;
                        mbMethodName += "::";
                        mbMethodName += CLASS_PROXY_CTOR_NAME;
                        
                        codeWriter.writeToFragment(NewStringf("if (!%s.IsConstructCall())\n", ARGS_VAR));
                        codeWriter.writeToFragment("{\n");
                        {
                            Indenter ind2(m_indentationLevels);
                            codeWriter.writeToFragment(NewStringf("return v8::ThrowException(v8::Exception::TypeError(v8::String::New(\"Use the new operator to create instances of %s\")));\n", clsName.c_str()));
                        }
                        codeWriter.writeToFragment("}\n");

                        //wrap constructor
                        codeWriter.writeToFragment("//Check if this is a C++ class wrap constructor call\n");
                        codeWriter.writeToFragment(NewStringf("if (%s.Length() == 1 && %s[0]->IsExternal())\n", ARGS_VAR, ARGS_VAR));
                        codeWriter.writeToFragment("{\n");
                        {
                            Indenter ind2(m_indentationLevels);
                            codeWriter.writeToFragment(NewStringf("%s* obj = (%s*)v8::External::Unwrap(%s[0]);\n", proxyClsName.c_str(), proxyClsName.c_str(), ARGS_VAR));
                            codeWriter.writeToFragment("//Wrap to \"this\"\n");
                            codeWriter.writeToFragment(NewStringf("obj->Wrap(%s.This());\n", ARGS_VAR));
                            codeWriter.writeToFragment(NewStringf("return %s.This();\n", ARGS_VAR));
                        }
                        codeWriter.writeToFragment("}\n");

                        codeWriter.writeToFragment(NewStringf("Ptr<%s> %s;\n", clsName.c_str(), WRAPPED_PTR_VAR));
                        ExceptionCodeWriter writer(codeWriter, mbMethodName);
                        codeWriter.writeToFragment(m_ctorSetupCode[clsName]);
                        codeWriter.writeToFragment("\n");
                    }
                    codeWriter.writeToFragment(NewStringf("return v8::ThrowException(v8::Exception::Error(v8::String::New(\"Could not resolve suitable constructor for %s based on given arguments\")));\n", clsName.c_str()));
                }
                writeToFile(code);
            }
            else
            {
                Indenter ind(m_indentationLevels);
                //wrap constructor
                writeToFile("//Check if this is a C++ class wrap constructor call\n");
                writeToFile(NewStringf("if (%s.Length() == 1 && %s[0]->IsExternal())\n", ARGS_VAR, ARGS_VAR));
                writeToFile("{\n");
                {
                    Indenter ind2(m_indentationLevels);
                    writeToFile(NewStringf("%s* obj = (%s*)v8::External::Unwrap(%s[0]);\n", proxyClsName.c_str(), proxyClsName.c_str(), ARGS_VAR));
                    writeToFile("//Wrap to \"this\"\n");
                    writeToFile(NewStringf("obj->Wrap(%s.This());\n", ARGS_VAR));
                    writeToFile(NewStringf("return %s.This();\n", ARGS_VAR));
                }
                writeToFile("}\n");
                writeToFile(NewStringf("return v8::ThrowException(v8::Exception::Error(v8::String::New(\"Class %s has no public constructors\")));\n", clsName.c_str()));
            }
            writeToFile("}\n\n");
        }

        writeToFile("//=====================================\n");
        writeToFile("//Proxy class setup methods\n");
        writeToFile("//=====================================\n");
        for (StringList::iterator it = m_classSetupCalls.begin(); it != m_classSetupCalls.end(); it++)
        {
            std::string call = (*it);
            writeToFile(call);
            writeToFile("\n");
        }

        writeToFile(NewStringf("extern \"C\" void init(v8::Handle<v8::Object> %s) {\n", EXPORTS_VAR));
        {
            Indenter ind(m_indentationLevels);
            for (StringList::iterator it = m_classExportCalls.begin(); it != m_classExportCalls.end(); it++)
            {
                std::string call = (*it);
                writeToFile(call);
                writeToFile("\n");
            }

            for (StringList::iterator it = m_classInheritCalls.begin(); it != m_classInheritCalls.end(); it++)
            {
                std::string call = (*it);
                writeToFile(call);
                writeToFile("\n");
            }
        }
        writeToFile("}\n");
        writeToFile(NewStringf("NODE_MODULE(%s, init)\n", modName));
        return SWIG_OK;
    }

    virtual int functionWrapper(Node* n)
    {
        String* name    = Getattr(n,"sym:name");
        SwigType *type  = Getattr(n,"type");
        ParmList *parms = Getattr(n,"parms");

        String* entry = NewString("MgInitializeWebTier");
        String* ctorPrefix = NewString("new_");
        String* dtorPrefix = NewString("delete_");
        if (Strstr(name, entry))
        {
            std::string globalFuncFragment;
            FragmentWriter cls(m_indentationLevels, globalFuncFragment);
            cls.writeToFragment(NewStringf("//Global Function: %s\n", name));
            std::string proxyFuncName;
            setProxyClassName(name, proxyFuncName);
            cls.writeToFragment(NewStringf("static v8::Handle<v8::Value> %s(const v8::Arguments& %s) {\n", proxyFuncName.c_str(), ARGS_VAR));
            {
                Indenter ind(m_indentationLevels);
                {
                    ExceptionCodeWriter writer(cls, proxyFuncName);
                    FunctionCallSetup setup;
                    std::string cleanup;
                    getFunctionArgumentAssignmentCode(parms, proxyFuncName, cls, &setup);
                    cls.writeToFragment(NewStringf("%s(%s);\n", name, setup.getFunctionArgumentString()));
                }
                cls.writeToFragment("return v8::Undefined();\n");
            }
            cls.writeToFragment("}\n");

            m_classMethodDecls.push_back(globalFuncFragment);
            std::string initExportCode;
            initExportCode += EXPORTS_VAR;
            initExportCode += "->Set(v8::String::NewSymbol(\"";
            initExportCode += Char(name);
            initExportCode += "\"), v8::FunctionTemplate::New(";
            initExportCode += proxyFuncName;
            initExportCode += ")->GetFunction());\n";
            m_classExportCalls.push_back(initExportCode);
        }
        else if (Strstr(name, ctorPrefix))
        {
            std::string ctorFragment;
            FragmentWriter ctor(m_indentationLevels, ctorFragment);
            String* ovName = getOverloadedName(n);
            ctor.writeToFragment(NewStringf("//ctor Function Wrapper: %s\n", name));

            std::string mbName = Char(name);
            //Strip the new_ prefix
            std::string ctorName = mbName.substr(4);
            
            std::string proxyClsName;
            setProxyClassName(ctorName.c_str(), proxyClsName);

            ctor.writeToFragment(NewStringf("static %s* %s(const v8::Arguments& %s) {\n", ctorName.c_str(), ovName, ARGS_VAR));
            {
                Indenter ind(m_indentationLevels);

                FunctionCallSetup setup;
                std::string mbOvName;
                mbOvName += Char(ovName);
                getFunctionArgumentAssignmentCode(parms, mbOvName, ctor, &setup, false);
                ctor.writeToFragment(NewStringf("Ptr<%s> retVal;\n", ctorName.c_str()));
                ctor.writeToFragment(NewStringf("retVal = new %s(%s);\n", ctorName.c_str(), setup.getFunctionArgumentString()));
                ctor.writeToFragment("return retVal.Detach();\n");

                std::string ctorFwdCode;
                const std::vector<ArgInfo>& args = setup.getArgs();
                //TODO: Naive implementation (arg count). Two constructor overloads with the same number of args will
                //break this.
                FragmentWriter ctorCheck(m_indentationLevels, ctorFwdCode);
                ctorCheck.writeToFragment(NewStringf("//Checking args suitability for method: %s\n", ovName));
                ctorCheck.writeToFragment(NewStringf("if (%s.Length() == %d)\n", ARGS_VAR, args.size()));
                ctorCheck.writeToFragment("{\n");
                {
                    Indenter ind2(m_indentationLevels);
                    ctorCheck.writeToFragment("//Forward ctor call\n");
                    ctorCheck.writeToFragment(NewStringf("%s = %s(args);\n", WRAPPED_PTR_VAR, ovName));
                    ctorCheck.writeToFragment(NewStringf("%s* proxy = new %s(%s);\n", proxyClsName.c_str(), proxyClsName.c_str(), WRAPPED_PTR_VAR));
                    ctorCheck.writeToFragment(NewStringf("proxy->Wrap(%s.This());\n", ARGS_VAR));
                    ctorCheck.writeToFragment(NewStringf("return %s.This();\n", ARGS_VAR));
                }
                ctorCheck.writeToFragment("}\n");

                if (m_ctorSetupCode.find(ctorName) == m_ctorSetupCode.end())
                {
                    m_ctorSetupCode[ctorName] = ctorFwdCode;
                }
                else
                {
                    std::string& code = m_ctorSetupCode[ctorName];
                    code += "\n";
                    code += ctorFwdCode;
                }
            }
            ctor.writeToFragment("}\n");
            m_classMethodDecls.push_back(ctorFragment);
        }
        else if (Strstr(name, dtorPrefix))
        {
            std::string dtorFragment;
            FragmentWriter dtor(m_indentationLevels, dtorFragment);
            dtor.writeToFragment(NewStringf("//dtor Function Wrapper: %s\n", name));
            dtor.writeToFragment(NewStringf("static v8::Handle<v8::Value> %s(const v8::Arguments& %s) {\n", name, ARGS_VAR));
            {
                Indenter ind(m_indentationLevels);

                FunctionCallSetup setup;
                std::string mbName;
                mbName += Char(name);
                getFunctionArgumentAssignmentCode(parms, mbName, dtor, &setup);
                
                //TODO: Should check if node/v8 allows this. This is theoretical atm
                //
                //There'll only be one argument, the pointer to release
                //Argument marshalling will create a Ptr<> of the internal pointer
                //and a Proxy pointer that wraps another Ptr<> of the same pointer
                //
                //Thus deleting the proxy pointer will trigger the internal Ptr<> to
                //release it
                dtor.writeToFragment("delete proxyArg0;\n");
            }
            dtor.writeToFragment("}\n");
            m_classMethodDecls.push_back(dtorFragment);
        }
        Language::functionWrapper(n);
        return SWIG_OK;
    }

    void writeTabsToFile() 
    {
        if (m_indentationLevels > 0)
        {
            for (int i = 0; i < m_indentationLevels; i++)
            {
                Printf(m_fOutputFile, "\t");
            }
        }
    }
    
    void writeToFile(String* str, bool bDelete = true)
    {
        writeTabsToFile();
        Printf(m_fOutputFile, "%s", str);
        if (bDelete)
            Delete(str);
    }

    void writeToFile(const std::string& str)
    {
        writeTabsToFile();
        Printf(m_fOutputFile, str.c_str());
    }

    void writeToFile(const char* str)
    {
        writeTabsToFile();
        Printf(m_fOutputFile, str);
    }

    static void setProxyClassName(const char* clsName, std::string& sOut)
    {
        sOut = "Proxy";
        sOut += clsName;
    }

    static void setProxyClassName(String* clsName, std::string& sOut)
    {
        sOut = "Proxy";
        sOut += Char(clsName);
    }

    bool isCurrentClassMgObject()
    {
        String* clsCmp = NewString("MgObject");
        bool bRet = (Strcmp(clsCmp, m_currentClassName) == 0);
        Delete(clsCmp);
        return bRet;
    }

    virtual int classHandler(Node* n)
    {
        m_classMemberExportCalls.clear();
        m_currentClassSetupFragment = std::string();
        m_currentFragment = std::string();
        m_currentInheritFragment = std::string();
        m_currentExportFragment = std::string();
        m_currentClassName = NewString(Getattr(n,"sym:name"));

        String* baseClassName = NULL;
        List *baselist = Getattr(n,"bases");
        if (baselist) {
            Iterator base = First(baselist);
            baseClassName = NewString(Getattr(base.item,"name"));
        } else {
            baseClassName = NewString("");
        }

        FragmentWriter cls(m_indentationLevels, m_currentFragment);
        FragmentWriter clsSetup(m_indentationLevels, m_currentClassSetupFragment);

        //NOTE: We currently don't export MgException classes, they're unusable in JavaScript, any exceptions that
        //any MapGuide API will throw will be caught by this node.js extension and re-thrown as a JavaScript
        //Error object.
        if (!Strstr(m_currentClassName, "Exception"))
        {
            cls.writeToFragment(NewStringf("//Node.js wrapper for class: %s (inherits from %s)\n", m_currentClassName, baseClassName));

            std::string clsName;
            clsName += Char(m_currentClassName);

            std::string proxyClsName;
            setProxyClassName(m_currentClassName, proxyClsName);
            
            std::string clsHead = "class "; 
            clsHead += proxyClsName;
            clsHead += " : node::ObjectWrap\n";
            
            //Export call
            m_currentExportFragment += proxyClsName;
            m_currentExportFragment += "::Export(";
            m_currentExportFragment += EXPORTS_VAR;
            m_currentExportFragment += ");";

            //Unmanaged pointer member
            cls.writeToFragment(clsHead);
            cls.writeToFragment("{\n");
            cls.writeToFragment("protected:\n");
            
            bool isMgObject = isCurrentClassMgObject();

            std::string ptrDecl;
            ptrDecl += "\t";
            ptrDecl += Char(m_currentClassName);
            ptrDecl += "* m_ptr;\n";
            cls.writeToFragment(ptrDecl);

            cls.writeToFragment("public:\n");
            std::string ctorDecl;
            ctorDecl += proxyClsName;
            ctorDecl += "(";
            ctorDecl += Char(m_currentClassName);
            ctorDecl += "* ptr)\n";
            //ctor
            {
                Indenter ind(m_indentationLevels);
                cls.writeToFragment(ctorDecl);
                cls.writeToFragment("{\n");
                if (isMgObject) //Anything inheriting from MgObject will be MgDisposable
                    cls.writeToFragment("\tm_ptr = SAFE_ADDREF((MgDisposable*)ptr);\n");
                else
                    cls.writeToFragment("\tm_ptr = SAFE_ADDREF(ptr);\n");
                cls.writeToFragment("}\n");
            }
            std::string dtorDecl;
            dtorDecl += "~";
            dtorDecl += proxyClsName;
            dtorDecl += "()\n";
            //dtor
            {
                Indenter ind(m_indentationLevels);
                cls.writeToFragment(dtorDecl);
                cls.writeToFragment("{\n");
                if (isMgObject) //Anything inheriting from MgObject will be MgDisposable 
                    cls.writeToFragment("\t((MgDisposable*)m_ptr)->Release();\n");
                else
                    cls.writeToFragment("\tSAFE_RELEASE(m_ptr);\n");

                if (m_bInsertDebuggingCode)
                    cls.writeToFragment(NewStringf("\tprintf(\"Released instance of %s\\n\");\n", clsName.c_str()));
                cls.writeToFragment("}\n");
            }

            //Stubs to be implemented further down
            {
                Indenter ind(m_indentationLevels);
                cls.writeToFragment("static v8::Handle<v8::Value> CreateJsProxy(MgObject* ptr);\n"); 
                cls.writeToFragment("static void Export(v8::Handle<v8::Object> exports);\n");
            }

            //Class body
            {
                Indenter ind(m_indentationLevels);
                Language::classHandler(n);
            }

            //Internal pointer accessor
            {
                Indenter ind(m_indentationLevels);

                cls.writeToFragment("//Sets the internal pointer. Pointer will be AddRef()'d. Existing pointer (if any) will be Release()'d\n");
                if (isMgObject)
                    cls.writeToFragment(NewStringf("void SetInternalPointer(%s* ptr) { if (m_ptr) { ((MgDisposable*)m_ptr)->Release(); }; m_ptr = SAFE_ADDREF((MgDisposable*)m_ptr); }\n", m_currentClassName));
                else
                    cls.writeToFragment(NewStringf("void SetInternalPointer(%s* ptr) { SAFE_RELEASE(m_ptr); m_ptr = SAFE_ADDREF(m_ptr); }\n", m_currentClassName));

                cls.writeToFragment("//Returns the internal pointer. Pointer is already AddRef()'d and requires release by the caller when done (automatically if assigned to a Ptr<>)\n");
                if (isMgObject)
                    cls.writeToFragment(NewStringf("%s* GetInternalPointer() { return SAFE_ADDREF((MgDisposable*)m_ptr); }\n", m_currentClassName));
                else
                    cls.writeToFragment(NewStringf("%s* GetInternalPointer() { return SAFE_ADDREF(m_ptr); }\n", m_currentClassName));
            }
            
            //Function template to apply inheritance
            {
                Indenter ind(m_indentationLevels);
                cls.writeToFragment(NewStringf("static v8::Persistent<v8::FunctionTemplate> constructor_tpl;\n"));
            }

            cls.writeToFragment("private:\n");
            //Proxy Constructor
            {
                Indenter ind(m_indentationLevels);
                cls.writeToFragment(NewStringf("static v8::Handle<v8::Value> %s(const v8::Arguments& %s);\n", CLASS_PROXY_CTOR_NAME, ARGS_VAR));
                cls.writeToFragment(NewStringf("static v8::Persistent<v8::Function> constructor;\n"));
            }

            //node/v8 wrap mg pointer factory method
            {
                Indenter ind(m_indentationLevels);
                clsSetup.writeToFragment(NewStringf("v8::Handle<v8::Value> %s::CreateJsProxy(MgObject* ptr)\n", proxyClsName.c_str()));
                clsSetup.writeToFragment("{\n");
                {
                    Indenter ind2(m_indentationLevels);
                    clsSetup.writeToFragment("v8::HandleScope scope;\n");
                    clsSetup.writeToFragment(NewStringf("%s* proxyVal = new %s((%s*)ptr);\n", proxyClsName.c_str(), proxyClsName.c_str(), m_currentClassName));
                    clsSetup.writeToFragment("//Invoke the \"wrap\" constructor\n");
                    clsSetup.writeToFragment("v8::Handle<v8::Value> argv[1] = { v8::External::New(proxyVal) };\n");
                    clsSetup.writeToFragment("v8::Local<v8::Object> instance = constructor->NewInstance(1, argv);\n");
                    clsSetup.writeToFragment("return scope.Close(instance);\n");
                }
                clsSetup.writeToFragment("}\n");
            }

            //node/v8 export function
            {
                Indenter ind(m_indentationLevels);
                clsSetup.writeToFragment(NewStringf("void %s::Export(v8::Handle<v8::Object> %s)\n", proxyClsName.c_str(), EXPORTS_VAR));
                clsSetup.writeToFragment("{\n");
                {
                    Indenter ind2(m_indentationLevels);
                    clsSetup.writeToFragment(NewStringf("v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(%s::%s);\n", proxyClsName.c_str(), CLASS_PROXY_CTOR_NAME));
                    clsSetup.writeToFragment(NewStringf("%s = v8::Persistent<v8::FunctionTemplate>::New(tpl);\n", CLASS_EXPORT_VAR));

                    //This is ok because export calls are ordered with base classes first, so we're assured to be
                    //inheriting from an initialized base class function template
                    if (Strcmp("", baseClassName) != 0)
                    {
                        std::string proxyBaseClsName;
                        setProxyClassName(baseClassName, proxyBaseClsName);
                        clsSetup.writeToFragment("//Inherit from base class function template\n");
                        clsSetup.writeToFragment(NewStringf("constructor_tpl->Inherit(%s::constructor_tpl);\n", proxyBaseClsName.c_str()));
                    }

                    clsSetup.writeToFragment("//For the internal pointer\n");
                    clsSetup.writeToFragment(NewStringf("%s->InstanceTemplate()->SetInternalFieldCount(1);\n", CLASS_EXPORT_VAR));
                    clsSetup.writeToFragment(NewStringf("%s->SetClassName(v8::String::NewSymbol(\"%s\"));\n", CLASS_EXPORT_VAR, m_currentClassName));

                    std::string ctorReg;
                    ctorReg += proxyClsName;
                    ctorReg += "::constructor = v8::Persistent<v8::Function>::New(";
                    ctorReg += CLASS_EXPORT_VAR;
                    ctorReg += "->GetFunction());\n";

                    std::string ctorReg2;
                    ctorReg2 += EXPORTS_VAR;
                    ctorReg2 += "->Set(v8::String::NewSymbol(\"";
                    ctorReg2 += Char(m_currentClassName);
                    ctorReg2 += "\"), ";
                    ctorReg2 += proxyClsName;
                    ctorReg2 += "::constructor);\n";

                    m_classMemberExportCalls.push_back(ctorReg);
                    m_classMemberExportCalls.push_back(ctorReg2);

                    for (StringList::iterator it = m_classMemberExportCalls.begin(); it != m_classMemberExportCalls.end(); it++)
                    {
                        clsSetup.writeToFragment((*it));
                        clsSetup.writeToFragment("\n");
                    }
                }
                clsSetup.writeToFragment("}\n");
            }

            cls.writeToFragment("};");

            Delete(m_currentClassName);
            m_currentClassName = NULL;

            m_classDecls[clsName] = m_currentFragment;
            m_classExportCalls.push_back(m_currentExportFragment);
            m_classSetupCalls.push_back(m_currentClassSetupFragment);

        }
        Delete(baseClassName);
        return SWIG_OK;
    }

    virtual int memberconstantHandler(Node* n)
    {
        String* name = Getattr(n, "sym:name");
        FragmentWriter frag(m_indentationLevels, m_currentFragment);
        frag.writeToFragment(NewStringf("//\tMember Constant: %s::%s\n", m_currentClassName, name));
        String* swExpCall = NewStringf("//TODO: Export constant %s::%s", m_currentClassName, name);
        std::string expCall = Char(swExpCall);
        Delete(swExpCall);
        m_classMemberExportCalls.push_back(expCall);
        return SWIG_OK;
    }

    virtual int memberfunctionHandler(Node* n)
    {
        String* name = Getattr(n, "sym:name");
        SwigType *retType  = Getattr(n,"type");
        SwigType *retTypeUnprefixed = SwigType_base(retType);
        String* overloaded_name = Copy(getOverloadedName(n));
        String* overloaded_name_qualified = Swig_name_member(m_currentClassName, overloaded_name);
        ParmList *parms  = Getattr(n,"parms");

        std::string proxyClsName;
        setProxyClassName(m_currentClassName, proxyClsName);
        
        bool isMgPointer = !SwigType_issimple(retType);
        bool isVoid = (Strcmp(retType, "void") == 0);

        m_currentMethodFragment = std::string();

        FragmentWriter meth(m_indentationLevels, m_currentMethodFragment);
        //Proxy method is encoded as ClassName_SwigQualifiedMethodName

        std::string mbName;
        std::string mbOvName;
        mbName += Char(name);
        mbOvName += Char(overloaded_name);

        std::string clsName;
        clsName += Char(m_currentClassName);

        String* name_qualified = Swig_name_member(m_currentClassName, name);
        std::string ovMethodName;
        ovMethodName += Char(name_qualified);

        FunctionCallSetup setup;

        meth.writeToFragment(NewStringf("//\tMember Function: %s::%s\n", m_currentClassName, overloaded_name));
        meth.writeToFragment(NewStringf("static v8::Handle<v8::Value> %s(const v8::Arguments& %s) {\n", overloaded_name_qualified, ARGS_VAR));
        {
            Indenter ind(m_indentationLevels);

            std::string wrappedPtrCall;
            meth.writeToFragment(NewStringf("//Return type - %s (pointer? %s)\n", retType, (isMgPointer ? "true" : "false")));
            
            if (isVoid)
            {
                meth.writeToFragment("//Function is void and does not return a value\n");
            }
            else
            {
                if (isMgPointer)
                {
                    meth.writeToFragment(NewStringf("Ptr<%s> %s;\n", retTypeUnprefixed, FUNC_RET_VAR));
                }
                else
                {
                    meth.writeToFragment(NewStringf("%s %s;\n", retType, FUNC_RET_VAR));
                }
            }

            if (!isVoid)
            {
                wrappedPtrCall += FUNC_RET_VAR;
                wrappedPtrCall += " = ";
            }
            wrappedPtrCall += WRAPPED_PTR_VAR;
            wrappedPtrCall += "->";
            
            if (isCurrentClassMgObject())
                meth.writeToFragment(NewStringf("%s* %s = NULL;\n", m_currentClassName, WRAPPED_PTR_VAR));
            else
                meth.writeToFragment(NewStringf("Ptr<%s> %s;\n", m_currentClassName, WRAPPED_PTR_VAR));
            
            
            {
                std::string mbMethodName = Char(overloaded_name_qualified);
                ExceptionCodeWriter writer(meth, mbMethodName, "MG_TRY", "MG_CATCH", isCurrentClassMgObject());   
                {
                    Indenter ind2(m_indentationLevels);
                    if (parms)
                    {   
                        std::string mbQName;
                        mbQName += Char(overloaded_name_qualified);
                        getFunctionArgumentAssignmentCode(parms, mbQName, meth, &setup);
                        wrappedPtrCall += Char(name);
                        wrappedPtrCall += "(";
                        wrappedPtrCall += setup.getFunctionArgumentString();
                        wrappedPtrCall += ");\n";
                    }
                    else
                    {
                        meth.writeToFragment("//Function has no arguments\n");
                        wrappedPtrCall += Char(name);
                        wrappedPtrCall += "();\n";
                    }
                    
                    meth.writeToFragment(NewStringf("%s* proxy = node::ObjectWrap::Unwrap<%s>(%s.This());\n", proxyClsName.c_str(), proxyClsName.c_str(), ARGS_VAR));
                    meth.writeToFragment(NewStringf("%s = proxy->GetInternalPointer();\n", WRAPPED_PTR_VAR));
                    
                    //Here's the actual call
                    meth.writeToFragment("//Do your thing\n");
                    meth.writeToFragment(wrappedPtrCall);
                    meth.writeToFragment("\n");

                }
            }
            if (isCurrentClassMgObject())
                meth.writeToFragment(NewStringf("if (%s) { ((MgDisposable*)%s)->Release(); %s = NULL; }\n", WRAPPED_PTR_VAR, WRAPPED_PTR_VAR, WRAPPED_PTR_VAR));
            
            meth.writeToFragment("\n");
            meth.writeToFragment("//Return value to javascript\n");
            if (isVoid)
            {
                meth.writeToFragment("return v8::Undefined();\n");
            }
            else
            {
                if (isMgPointer) //Stub
                {
                    meth.writeToFragment("v8::HandleScope scope;\n");
                    meth.writeToFragment(NewStringf("v8::Handle<v8::Value> instance = JsProxyFactory::CreateJsProxy(%s);\n", FUNC_RET_VAR));
                    meth.writeToFragment("return scope.Close(instance);\n");
                }
                else
                {
                    std::string scopeCloseCall;
                    std::string preCloseCallCode;
                    meth.writeToFragment("v8::HandleScope scope;\n");
                    if (isMgPointer)
                    {
                         
                    }
                    else
                    {
                        getScopeCloseCallForBasicType(retType, scopeCloseCall, preCloseCallCode);
                    }
                    if (!preCloseCallCode.empty())
                        meth.writeToFragment(preCloseCallCode);
                    meth.writeToFragment(NewStringf("return scope.Close(%s);\n", scopeCloseCall.c_str()));
                }
            }
        }
        meth.writeToFragment("}\n");

        //Check if this is an overload
        bool bIsOverload = (Strcmp(name, overloaded_name) != 0);
        if (bIsOverload)
        {
            const std::vector<ArgInfo>& args = setup.getArgs();

            std::string overloadResFrag;
            FragmentWriter ovRes(m_indentationLevels, overloadResFrag);
            
            ovRes.writeToFragment(NewStringf("//Checking args suitability for method: %s\n", overloaded_name_qualified));
            ovRes.writeToFragment(NewStringf("if (%s.Length() == %d)\n", ARGS_VAR, args.size()));
            ovRes.writeToFragment("{\n");
            {
                Indenter ind2(m_indentationLevels);
                std::string condition;
                writeArgumentsMatchCondition(args, condition);
                if (!condition.empty())
                {
                    //We need to add an argument check before invocation
                    ovRes.writeToFragment(NewStringf("if (%s)\n", condition.c_str()));
                    ovRes.writeToFragment("{\n");
                    {
                        Indenter ind3(m_indentationLevels);
                        ovRes.writeToFragment("//Forward method call\n");
                        if (isVoid)
                        {
                            ovRes.writeToFragment(NewStringf("%s(args);\n", overloaded_name_qualified));
                            ovRes.writeToFragment("return v8::Undefined();\n");
                        }
                        else
                        {
                            ovRes.writeToFragment(NewStringf("return %s(args);\n", overloaded_name_qualified));
                        }
                    }
                    ovRes.writeToFragment("}\n");
                }
                else
                {
                    ovRes.writeToFragment("//Forward method call\n");
                    if (isVoid)
                    {
                        ovRes.writeToFragment(NewStringf("%s(args);\n", overloaded_name_qualified));
                        ovRes.writeToFragment("return v8::Undefined();\n");
                    }
                    else
                    {
                        ovRes.writeToFragment(NewStringf("return %s(args);\n", overloaded_name_qualified));
                    }
                }
            }
            ovRes.writeToFragment("}\n\n");

            //Append to method resolution code
            if (m_classMethodOverloadResolution.find(ovMethodName) == m_classMethodOverloadResolution.end())
            {
                m_classMethodOverloadResolution[ovMethodName] = overloadResFrag;
            }
            else
            {
                std::string& frag = m_classMethodOverloadResolution[ovMethodName];
                frag += overloadResFrag;
            }
        }

        if (bIsOverload)
        {
            //Have we registered this as a method with multiple overloads?
            if (m_classMethodOverloads.find(clsName) == m_classMethodOverloads.end())
            {
                m_classMethodOverloads[clsName] = StringSet();
            }
            if (m_classMethodOverloads[clsName].find(ovMethodName) == m_classMethodOverloads[clsName].end())
            {
                String* regCall = NewStringf(
                    "%s->PrototypeTemplate()->Set(v8::String::NewSymbol(\"%s\"), v8::FunctionTemplate::New(%s)->GetFunction());",
                    CLASS_EXPORT_VAR,
                    name,
                    name_qualified);
                std::string sRegCall;
                sRegCall += Char(regCall);
                m_classMemberExportCalls.push_back(sRegCall);
                Delete(regCall);

                m_classMethodOverloads[clsName].insert(ovMethodName);
            }
        }
        else
        {
            String* regCall = NewStringf(
                "%s->PrototypeTemplate()->Set(v8::String::NewSymbol(\"%s\"), v8::FunctionTemplate::New(%s)->GetFunction());",
                CLASS_EXPORT_VAR,
                overloaded_name,
                overloaded_name_qualified);
            std::string sRegCall;
            sRegCall += Char(regCall);
            m_classMemberExportCalls.push_back(sRegCall);
            Delete(regCall);
        }
        m_classMethodDecls.push_back(m_currentMethodFragment);
        
        Delete(overloaded_name);
        return SWIG_OK;
    }

    void writeArgumentsMatchCondition(const std::vector<ArgInfo>& args, std::string& condition)
    {
        for (std::vector<ArgInfo>::const_iterator it = args.begin(); it != args.end(); it++)
        {
            if (it->isMgPointer)
            {
                if (condition.empty())
                {
                    String* str = NewStringf("%s[%d]->IsObject()", ARGS_VAR, it->offset);
                    condition += Char(str);
                    Delete(str);
                }
                else
                {
                    String* str = NewStringf(" && %s[%d]->IsObject()", ARGS_VAR, it->offset);
                    condition += Char(str);
                    Delete(str);
                }
            }
            else
            {
                const char* type = it->type.c_str();
                if (Strcmp(type, "BYTE") == 0 ||
                    Strcmp(type, "INT8") == 0 ||
                    Strcmp(type, "INT16") == 0 ||
                    Strcmp(type, "INT32") == 0 ||
                    Strcmp(type, "STATUS") == 0 ||                    //Huh?
                    Strcmp(type, "MgSiteInfo::MgSiteStatus") == 0 ||  //Huh? I thought MapGuide API doesn't expose enums!
                    Strcmp(type, "MgSiteInfo::MgPortType") == 0 ||    //Huh? I thought MapGuide API doesn't expose enums!
                    Strcmp(type, "int") == 0 ||
                    Strcmp(type, "UINT32") == 0 ||
                    Strcmp(type, "INT64") == 0 ||
                    Strcmp(type, "long") == 0 ||
                    Strcmp(type, "long long") == 0)
                {
                    if (condition.empty())
                    {
                        String* str = NewStringf("%s[%d]->IsInt32()", ARGS_VAR, it->offset);
                        condition += Char(str);
                        Delete(str);
                    }
                    else
                    {
                        String* str = NewStringf(" && %s[%d]->IsInt32()", ARGS_VAR, it->offset);
                        condition += Char(str);
                        Delete(str);
                    }
                }
                else if (Strcmp(type, "double") == 0 ||
                         Strcmp(type, "float") == 0)
                {
                    if (condition.empty())
                    {
                        String* str = NewStringf("%s[%d]->IsNumber()", ARGS_VAR, it->offset);
                        condition += Char(str);
                        Delete(str);
                    }
                    else
                    {
                        String* str = NewStringf(" && %s[%d]->IsNumber()", ARGS_VAR, it->offset);
                        condition += Char(str);
                        Delete(str);
                    }
                }
                else if (Strcmp(type, "BYTE_ARRAY_IN") == 0 ||
                         Strcmp(type, "BYTE_ARRAY_OUT") == 0)
                {
                    if (condition.empty())
                    {
                        String* str = NewStringf("node::Buffer::HasInstance(%s[%d])", ARGS_VAR, it->offset);
                        condition += Char(str);
                        Delete(str);
                    }
                    else
                    {
                        String* str = NewStringf(" && node::Buffer::HasInstance(%s[%d])", ARGS_VAR, it->offset);
                        condition += Char(str);
                        Delete(str);
                    }
                }
                else if (Strcmp(type, "STRINGPARAM") == 0)
                {
                    if (condition.empty())
                    {
                        String* str = NewStringf("%s[%d]->IsString()", ARGS_VAR, it->offset);
                        condition += Char(str);
                        Delete(str);
                    }
                    else
                    {
                        String* str = NewStringf(" && %s[%d]->IsString()", ARGS_VAR, it->offset);
                        condition += Char(str);
                        Delete(str);
                    }
                }
                else if (Strcmp(type, "bool") == 0)
                {
                    if (condition.empty())
                    {
                        String* str = NewStringf("%s[%d]->IsBoolean()", ARGS_VAR, it->offset);
                        condition += Char(str);
                        Delete(str);
                    }
                    else
                    {
                        String* str = NewStringf(" && %s[%d]->IsBoolean()", ARGS_VAR, it->offset);
                        condition += Char(str);
                        Delete(str);
                    }
                }
            }
        }
    }

    void writeArgumentCheckFragment(FragmentWriter& frag, std::string& callingMethodName, String* name, SwigType* type, int argNo, bool isArgMgPointer, bool bThrowV8Exceptions = true)
    {
        if (isArgMgPointer)
        {
            if (bThrowV8Exceptions)
            {
                frag.writeToFragment(NewStringf("if (!%s[%d]->IsObject()) return v8::ThrowException(v8::Exception::Error(v8::String::New(\"Argument %d (%s: %s) is not an object\")));\n", ARGS_VAR, argNo, (argNo + 1), name, type));
            }
            else
            {
                frag.writeToFragment(NewStringf("if (!%s[%d]->IsObject())\n", ARGS_VAR, argNo));
                frag.writeToFragment("{\n");
                {
                    Indenter ind(m_indentationLevels);
                    frag.writeToFragment("MgStringCollection exArgs;\n");
                    frag.writeToFragment(NewStringf("exArgs.Add(L\"Argument %d (%s: %s) is not an object\");\n", (argNo + 1), name, type));
                    frag.writeToFragment(NewStringf("throw new MgInvalidArgumentException(L\"%s\", __LINE__, __WFILE__, NULL, L\"MgFormatInnerExceptionMessage\", &exArgs);\n", callingMethodName.c_str()));
                }
                frag.writeToFragment("}\n");
            }
        }
        else
        {
            //Write definition check
            if (bThrowV8Exceptions)
            {
                frag.writeToFragment(NewStringf("if (%s[%d]->IsUndefined()) return v8::ThrowException(v8::Exception::Error(v8::String::New(\"Argument %d (%s: %s) is undefined\")));\n", ARGS_VAR, argNo, (argNo + 1), name, type));
            }
            else
            {
                frag.writeToFragment(NewStringf("if (%s[%d]->IsUndefined())\n", ARGS_VAR, argNo));
                frag.writeToFragment("{\n");
                {
                    Indenter ind(m_indentationLevels);
                    frag.writeToFragment("MgStringCollection exArgs;\n");
                    frag.writeToFragment(NewStringf("exArgs.Add(L\"Argument %d (%s: %s) is undefined\");\n", (argNo + 1), name, type));
                    frag.writeToFragment(NewStringf("throw new MgInvalidArgumentException(L\"%s\", __LINE__, __WFILE__, NULL, L\"MgFormatInnerExceptionMessage\", &exArgs);\n", callingMethodName.c_str()));
                }
                frag.writeToFragment("}\n");
            }
            //Then type check
            if (Strcmp(type, "BYTE") == 0 ||
                Strcmp(type, "INT8") == 0 ||
                Strcmp(type, "INT16") == 0 ||
                Strcmp(type, "INT32") == 0 ||
                Strcmp(type, "STATUS") == 0 ||                    //Huh?
                Strcmp(type, "MgSiteInfo::MgSiteStatus") == 0 ||  //Huh? I thought MapGuide API doesn't expose enums!
                Strcmp(type, "MgSiteInfo::MgPortType") == 0 ||    //Huh? I thought MapGuide API doesn't expose enums!
                Strcmp(type, "int") == 0 ||
                Strcmp(type, "UINT32") == 0 ||
                Strcmp(type, "INT64") == 0 ||
                Strcmp(type, "long") == 0 ||
                Strcmp(type, "long long") == 0)
            {
                if (bThrowV8Exceptions)
                {
                    frag.writeToFragment(NewStringf("if (!%s[%d]->IsInt32()) return v8::ThrowException(v8::Exception::Error(v8::String::New(\"Argument %d (%s: %s) is not a number\")));\n", ARGS_VAR, argNo, (argNo + 1), name, type));
                }
                else
                {
                    frag.writeToFragment(NewStringf("if (!%s[%d]->IsInt32())\n", ARGS_VAR, argNo));
                    frag.writeToFragment("{\n");
                    {
                        Indenter ind(m_indentationLevels);
                        frag.writeToFragment("MgStringCollection exArgs;\n");
                        frag.writeToFragment(NewStringf("exArgs.Add(L\"Argument %d (%s: %s) is not a number\");\n", (argNo + 1), name, type));
                        frag.writeToFragment(NewStringf("throw new MgInvalidArgumentException(L\"%s\", __LINE__, __WFILE__, NULL, L\"MgFormatInnerExceptionMessage\", &exArgs);\n", callingMethodName.c_str()));
                    }
                    frag.writeToFragment("}\n");
                }
            }
            else if (Strcmp(type, "double") == 0 ||
                     Strcmp(type, "float") == 0)
            {
                if (bThrowV8Exceptions)
                {
                    frag.writeToFragment(NewStringf("if (!%s[%d]->IsNumber()) return v8::ThrowException(v8::Exception::Error(v8::String::New(\"Argument %d (%s: %s) is not a number\")));\n", ARGS_VAR, argNo, (argNo + 1), name, type));
                }
                else
                {
                    frag.writeToFragment(NewStringf("if (!%s[%d]->IsNumber())\n", ARGS_VAR, argNo));
                    frag.writeToFragment("{\n");
                    {
                        Indenter ind(m_indentationLevels);
                        frag.writeToFragment("MgStringCollection exArgs;\n");
                        frag.writeToFragment(NewStringf("exArgs.Add(L\"Argument %d (%s: %s) is not a number\");\n", (argNo + 1), name, type));
                        frag.writeToFragment(NewStringf("throw new MgInvalidArgumentException(L\"%s\", __LINE__, __WFILE__, NULL, L\"MgFormatInnerExceptionMessage\", &exArgs);\n", callingMethodName.c_str()));
                    }
                    frag.writeToFragment("}\n");
                }
            }
            else if (Strcmp(type, "BYTE_ARRAY_IN") == 0 ||
                     Strcmp(type, "BYTE_ARRAY_OUT") == 0)
            {
                if (bThrowV8Exceptions)
                {
                    frag.writeToFragment(NewStringf("if (!node::Buffer::HasInstance(%s[%d])) return v8::ThrowException(v8::Exception::Error(v8::String::New(\"Argument %d (%s: %s) is not a node Buffer\")));\n", ARGS_VAR, argNo, (argNo + 1), name, type));
                }
                else
                {
                    frag.writeToFragment(NewStringf("if (!node::Buffer::HasInstance(%s[%d]))\n", ARGS_VAR, argNo));
                    frag.writeToFragment("{\n");
                    {
                        Indenter ind(m_indentationLevels);
                        frag.writeToFragment("MgStringCollection exArgs;\n");
                        frag.writeToFragment(NewStringf("exArgs.Add(L\"Argument %d (%s: %s) is not a node Buffer\");\n", (argNo + 1), name, type));
                        frag.writeToFragment(NewStringf("throw new MgInvalidArgumentException(L\"%s\", __LINE__, __WFILE__, NULL, L\"MgFormatInnerExceptionMessage\", &exArgs);\n", callingMethodName.c_str()));
                    }
                    frag.writeToFragment("}\n");
                }
            }
            else if (Strcmp(type, "STRINGPARAM") == 0)
            {
                if (bThrowV8Exceptions)
                {
                    frag.writeToFragment(NewStringf("if (!%s[%d]->IsString()) return v8::ThrowException(v8::Exception::Error(v8::String::New(\"Argument %d (%s: %s) is not a string\")));\n", ARGS_VAR, argNo, (argNo + 1), name, type));
                }
                else
                {
                    frag.writeToFragment(NewStringf("if (!%s[%d]->IsString())\n", ARGS_VAR, argNo));
                    frag.writeToFragment("{\n");
                    {
                        Indenter ind(m_indentationLevels);
                        frag.writeToFragment("MgStringCollection exArgs;\n");
                        frag.writeToFragment(NewStringf("exArgs.Add(L\"Argument %d (%s: %s) is not a string\");\n", (argNo + 1), name, type));
                        frag.writeToFragment(NewStringf("throw new MgInvalidArgumentException(L\"%s\", __LINE__, __WFILE__, NULL, L\"MgFormatInnerExceptionMessage\", &exArgs);\n", callingMethodName.c_str()));
                    }
                    frag.writeToFragment("}\n");
                }
            }
            else if (Strcmp(type, "bool") == 0)
            {
                if (bThrowV8Exceptions)
                {
                    frag.writeToFragment(NewStringf("if (!%s[%d]->IsBoolean()) return v8::ThrowException(v8::Exception::Error(v8::String::New(\"Argument %d (%s: %s) is not a boolean\")));\n", ARGS_VAR, argNo, (argNo + 1), name, type));
                }
                else
                {
                    frag.writeToFragment(NewStringf("if (!%s[%d]->IsBoolean())\n", ARGS_VAR, argNo));
                    frag.writeToFragment("{\n");
                    {
                        Indenter ind(m_indentationLevels);
                        frag.writeToFragment("MgStringCollection exArgs;\n");
                        frag.writeToFragment(NewStringf("exArgs.Add(L\"Argument %d (%s: %s) is not a boolean\");\n", (argNo + 1), name, type));
                        frag.writeToFragment(NewStringf("throw new MgInvalidArgumentException(L\"%s\", __LINE__, __WFILE__, NULL, L\"MgFormatInnerExceptionMessage\", &exArgs);\n", callingMethodName.c_str()));
                    }
                    frag.writeToFragment("}\n");
                }
            }
        }
    }

    void writeArgumentAssignmentCode(FragmentWriter& frag, SwigType* type, SwigType* typeUnprefixed, String* name, int argNo, bool isArgMgPointer, bool bIsMgObject)
    {
        if (isArgMgPointer)
        {
            std::string proxyClsName;
            setProxyClassName(typeUnprefixed, proxyClsName);
            if (bIsMgObject)
            {
                frag.writeToFragment(NewStringf("Ptr<MgDisposable> arg%d = NULL;\n", argNo));
            }
            else
            {
                frag.writeToFragment(NewStringf("Ptr<%s> arg%d = NULL;\n", typeUnprefixed, argNo));
            }
            frag.writeToFragment(NewStringf("%s* proxyArg%d = NULL;\n", proxyClsName.c_str(), argNo));
            frag.writeToFragment(NewStringf("if (!%s[%d]->IsUndefined())\n", ARGS_VAR, argNo));
            frag.writeToFragment("{\n");
            {
                Indenter ind(m_indentationLevels);
                frag.writeToFragment(NewStringf("proxyArg%d = node::ObjectWrap::Unwrap<%s>(%s[%d]->ToObject());\n", argNo, proxyClsName.c_str(), ARGS_VAR, argNo));
                if (bIsMgObject)
                    frag.writeToFragment(NewStringf("arg%d = (MgDisposable*)proxyArg%d->GetInternalPointer();\n", argNo, argNo));
                else
                    frag.writeToFragment(NewStringf("arg%d = proxyArg%d->GetInternalPointer();\n", argNo, argNo));
            }
            frag.writeToFragment("}\n");
        }
        else
        {
            //Then type check
            if (Strcmp(type, "BYTE") == 0 ||
                Strcmp(type, "INT8") == 0 ||
                Strcmp(type, "INT16") == 0 ||
                Strcmp(type, "INT32") == 0 ||
                Strcmp(type, "STATUS") == 0 ||                    //Huh?
                Strcmp(type, "MgSiteInfo::MgSiteStatus") == 0 ||  //Huh? I thought MapGuide API doesn't expose enums!
                Strcmp(type, "MgSiteInfo::MgPortType") == 0 ||    //Huh? I thought MapGuide API doesn't expose enums!
                Strcmp(type, "int") == 0 ||
                Strcmp(type, "UINT32") == 0 ||
                Strcmp(type, "INT64") == 0 ||
                Strcmp(type, "long") == 0 ||
                Strcmp(type, "long long") == 0)
            {
                frag.writeToFragment(NewStringf("%s arg%d = (%s)%s[%d]->IntegerValue();\n", type, argNo, type, ARGS_VAR, argNo));
            }
            else if (Strcmp(type, "double") == 0 ||
                     Strcmp(type, "float") == 0)
            {
                frag.writeToFragment(NewStringf("%s arg%d = (%s)%s[%d]->NumberValue();\n", type, argNo, type, ARGS_VAR, argNo));
            }
            else if (Strcmp(type, "BYTE_ARRAY_OUT") == 0)
            {
                //frag.writeToFragment(NewStringf("std::string str%d = args[%d]->ToString();\n", argNo, argNo));
                //frag.writeToFragment(NewStringf("Ptr<MgByte> bytes%d = new MgByte((BYTE_ARRAY_IN)str%d);\n", argNo, argNo));
                //frag.writeToFragment(NewStringf("STRING arg%d = MgUtil::MultiByteToWideChar(args[%d]->ToString());\n", argNo, argNo));
                //frag.writeToFragment("//TODO: Case not handled: BYTE_ARRAY_OUT\n");
                frag.writeToFragment(NewStringf("BYTE_ARRAY_OUT arg%d = (BYTE_ARRAY_OUT)node::Buffer::Data(%s[%d]->ToObject());\n", argNo, ARGS_VAR, argNo));
            }
            else if (Strcmp(type, "BYTE_ARRAY_IN") == 0)
            {
                //frag.writeToFragment(NewStringf("std::string str%d = args[%d]->ToString();\n", argNo, argNo));
                //frag.writeToFragment(NewStringf("Ptr<MgByte> bytes%d = new MgByte((BYTE_ARRAY_IN)str%d);\n", argNo, argNo));
                //frag.writeToFragment(NewStringf("STRING arg%d = MgUtil::MultiByteToWideChar(args[%d]->ToString());\n", argNo, argNo));
                //frag.writeToFragment("//TODO: Case not handled: BYTE_ARRAY_IN\n");
                frag.writeToFragment(NewStringf("BYTE_ARRAY_IN arg%d = (BYTE_ARRAY_IN)node::Buffer::Data(%s[%d]->ToObject());\n", argNo, ARGS_VAR, argNo));
            }
            else if (Strcmp(type, "STRINGPARAM") == 0)
            {
                frag.writeToFragment(NewStringf("v8::String::Utf8Value mbArg%d(%s[%d]->ToString());\n", argNo, ARGS_VAR, argNo));
                frag.writeToFragment(NewStringf("STRING arg%d = MgUtil::MultiByteToWideChar(std::string(*mbArg%d));\n", argNo, argNo));
            }
            else if (Strcmp(type, "bool") == 0)
            {
                frag.writeToFragment(NewStringf("%s arg%d = %s[%d]->BooleanValue();\n", type, argNo, ARGS_VAR, argNo));
            }
        }
    }

    void getFunctionArgumentAssignmentCode(ParmList* parms, std::string& callingMethodName, FragmentWriter& frag, FunctionCallSetup* setup, bool bThrowV8Exceptions = true)
    {
        std::string nativeMethodArgsString;
        int argNo = 0;
        Parm *p;
        for (p = parms; p; p = nextSibling(p)) 
        {
            SwigType *type  = Getattr(p,"type");
            SwigType *typeUnprefixed = SwigType_base(type);
            String   *name  = Getattr(p,"name");
            //String   *value = Getattr(p,"value");

            bool isArgMgPointer = !SwigType_issimple(type);
            std::string sName;
            std::string sType;
            if (isArgMgPointer)
                sType += Char(typeUnprefixed);
            else
                sType += Char(type);
            if (name)
                sName += Char(name);

            setup->addArg(sName, sType, argNo, isArgMgPointer);
            
            frag.writeToFragment("//Argument\n");
            frag.writeToFragment(NewStringf("// Name - %s\n", name));
            frag.writeToFragment(NewStringf("// Type - %s\n", type));
            //frag.writeToFragment(NewStringf("// Value - %s\n", value));
            
            //Before we extract values from v8::Arguments, do basic checks on the parameters
            writeArgumentCheckFragment(frag, callingMethodName, name, type, argNo, isArgMgPointer, bThrowV8Exceptions);

            bool bIsMgObject = (Strcmp(typeUnprefixed, "MgObject") == 0);

            writeArgumentAssignmentCode(frag, type, typeUnprefixed, name, argNo, isArgMgPointer, bIsMgObject);
            
            frag.writeToFragment("\n");

            if (!nativeMethodArgsString.empty())
                nativeMethodArgsString += ", ";

            nativeMethodArgsString += "arg";
            
            std::stringstream out;
            out << argNo;
            nativeMethodArgsString += out.str();

            Delete(name);
            argNo++;
        }
        setup->setFunctionArgumentString(nativeMethodArgsString);
    }

    void getScopeCloseCallForBasicType(SwigType* type, std::string& str, std::string& preCloseCallCode)
    {
        std::string invocation = "(";
        invocation += FUNC_RET_VAR;
        invocation += ")";
        if (Strcmp(type, "BYTE") == 0 ||
            Strcmp(type, "INT8") == 0 ||
            Strcmp(type, "INT16") == 0 ||
            Strcmp(type, "INT32") == 0 ||
            Strcmp(type, "STATUS") == 0 ||                    //Huh?
            Strcmp(type, "MgSiteInfo::MgSiteStatus") == 0 ||  //Huh? I thought MapGuide API doesn't expose enums!
            Strcmp(type, "int") == 0 ||
            Strcmp(type, "UINT32") == 0 ||
            Strcmp(type, "INT64") == 0 ||
            Strcmp(type, "long") == 0 ||
            Strcmp(type, "long long") == 0)
        {
            str += "v8::Integer::New";
            str += invocation;
        }
        else if (Strcmp(type, "double") == 0 ||
                 Strcmp(type, "float") == 0)
        {
            str += "v8::Number::New";
            str += invocation;
        }
        else if (Strcmp(type, "STRING") == 0)
        {
            preCloseCallCode += "std::string mbRetVal = MgUtil::WideCharToMultiByte(";
            preCloseCallCode += FUNC_RET_VAR;
            preCloseCallCode += ");\n";

            str += "v8::String::New(";
            str += "mbRetVal.c_str())";
        }
        else if (Strcmp(type, "bool") == 0)
        {
            str += "v8::Boolean::New";
            str += invocation;
        }
    }

    virtual int membervariableHandler(Node* n)
    {
        String* name = Getattr(n, "sym:name");
        FragmentWriter frag(m_indentationLevels, m_currentFragment);
        frag.writeToFragment(NewStringf("//\tMember Variable: %s::%s\n", m_currentClassName, name));
        
        String* swExpCall = NewStringf("//TODO: Export member variable %s::%s", m_currentClassName, name);
        std::string expCall = Char(swExpCall);
        Delete(swExpCall);
        m_classMemberExportCalls.push_back(expCall);

        return SWIG_OK;
    }

    String *getOverloadedName(Node *n, bool bFullyQualified = false) 
    {
        String *overloaded_name = NewStringf("%s", Getattr(n,"sym:name"));

        if (Getattr(n,"sym:overloaded")) {
            Printv(overloaded_name, Getattr(n,"sym:overname"), NIL);
        }

        return overloaded_name;
    }
};

extern "C" Language * swig_nodejs(void) {
    return new NODEJS();
}