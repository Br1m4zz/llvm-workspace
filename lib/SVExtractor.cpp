#include "SVExtractor.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/IR/Module.h"


using namespace llvm;


static constexpr char PassName[] =
    "Extracting State Varables in program";


/*-----------------------------------------------------------
新PM下，调用我们的分析PASS。具体实现在runOnModule之中，这是一个封装。
-----------------------------------------------------------*/
 SVExtractor::Result SVExtractor::run (Module &Mod, ModuleAnalysisManager &MAM)
 {
     return runOnModule(Mod);
 }

/*-----------------------------------------------------------
具体实现我们的分析PASS实现。
遍历module中所有的全局变量。注意LLVM中的全局Value包含：全局变量、全局的别名、别名
这个迭代器返回的是程序中的全局变量的迭代器。
-----------------------------------------------------------*/
SVExtractor::Result SVExtractor::runOnModule(Module &Mod)
{
    Result SVs;

    for(auto it = Mod.global_begin(), it_end = Mod.global_end(); it != it_end; ++it)
    {
        GlobalVariable *gv = &*it;
        //状态变量会发生变化，不可能是常量。目前的判决标准是，非常量的全局变量是状态变量
        if (!gv->isConstant())
                SVs.push_back(gv);        
    }
    return SVs;
}

static void printSVs(raw_ostream &OS, Module &Mod, SVExtractor::Result &SVs)
    noexcept {
        if(SVs.empty())return;
        OS << "------------------------"<<"\n";
        OS << "State Variable in Module \" "<< Mod.getName() << "\":\n";
        OS << "------------------------"<<"\n";

        for(auto it= SVs.begin(), it_end=SVs.end(); it!=it_end; ++it)
        {
            GlobalVariable *sv = *it;
            OS<< format("%-20s \n",sv->getName().str().c_str());
        }
    }

/*-----------------------------------------------------------
新PM下，调用我们的转换PASS，
具体实现在printSVs之中，这是一个封装。
转换PASS的run的实现中，PassInfoMixin类通常需要返回PreservedAnalyses::all()
-----------------------------------------------------------*/
PreservedAnalyses SVExtractorPrinter::run(Module &Mod, ModuleAnalysisManager &MAM)
{
    //先调用分析PASS，SVExtractor，拿到分析结果。
    auto &SVs = MAM.getResult<SVExtractor>(Mod);
    //再调用打印函数
    printSVs(OS, Mod, SVs);
    return PreservedAnalyses::all();
}

/*-----------------------------------------------------------
具体实现我们的打印函数实现，

-----------------------------------------------------------*/


/*-----------------------------------------------------------
新PM下PASS注册
注册的实现
-----------------------------------------------------------*/
llvm::AnalysisKey SVExtractor::Key;

PassPluginLibraryInfo getSVEPluginInfo()
{
    return{
        LLVM_PLUGIN_API_VERSION,
        "SVExtractor",
        LLVM_VERSION_STRING,
        [](PassBuilder &PB)
        {
            // 注册本PASS所需的输入参数。执行的参数输入为 "opt -passes=print<SV>"
            PB.registerPipelineParsingCallback
            (
                [&](StringRef Name, ModulePassManager &MPM, ArrayRef<PassBuilder::PipelineElement>)
                {
                    if (Name.equals("print<SV>")){
                         MPM.addPass(SVExtractorPrinter(llvm::outs()));
                         return true;
                    }
                    return false;
                }
            );
            //注册实际的分析PASS "MAM.getResult<SVExtractor>(Mod)"
            PB.registerAnalysisRegistrationCallback
            (
                [](ModuleAnalysisManager &MAM) 
                {
                    MAM.registerPass([&]{return SVExtractor(); });
                }
            );
        }
    };
}

//这里全局声明，实现我们PASS的注册
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getSVEPluginInfo();
}