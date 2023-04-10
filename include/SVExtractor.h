#include "llvm/Pass.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Module.h"

//声明域名空间LLVM中要用到的类
namespace llvm {
class FCmpInst;
class Function;
class Module;
class raw_ostream;
}

/*-----------------------------------------------------------
新PM下，定义我们的分析类，继承分析接口AnalysisInfoMixin，需要提供分析PASS所需的API信息
包含执行我们分析的run函数定义，与结果的类型的声明
-----------------------------------------------------------*/
class SVExtractor : public llvm::AnalysisInfoMixin<SVExtractor>
{
    public:
    using Result = std::vector<llvm::GlobalVariable* >;
    //using Result2 = std::vector<llvm::DILocalVariable* >;注意只能用Result声明结果，Result1是不行的
    //这个函数是继承PassInfoMixin的run函数的声明实现。在新PM下，这是被调用的目标函数。目标函数内本质封装了原型函数
    Result run(llvm::Module &module, llvm::ModuleAnalysisManager &);
    //原型函数，可以改成旧PM的接口。这里不需要传入MAM参数
    Result runOnModule(llvm::Module &module);
    static bool isRequired() { return true; }

    private:
    //声明结构体是其友元，使得其可以访问该类的私有成员
    friend struct llvm::AnalysisInfoMixin<SVExtractor>; 
    //这里的key用于新PM的PASS注册
    static llvm::AnalysisKey Key;   
};

/*-----------------------------------------------------------
新PM下，定义我们的转换类，继承分析接口PassInfoMixin
包含我们对打印函数的重定向为OS
-----------------------------------------------------------*/
class SVExtractorPrinter : public llvm::PassInfoMixin<SVExtractorPrinter>
{
    public:
    explicit SVExtractorPrinter(llvm::raw_ostream &OutStream) : OS(OutStream){};
    llvm::PreservedAnalyses run(llvm::Module &module, llvm::ModuleAnalysisManager &MAM);
    static bool isRequired() { return true; }
    private:
    llvm::raw_ostream &OS;
};