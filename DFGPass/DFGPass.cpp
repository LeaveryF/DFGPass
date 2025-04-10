#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Use.h>
#include <llvm/IR/Value.h>
#include <llvm/Pass.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>

using namespace llvm;

namespace {
struct DFGPass : public FunctionPass {
public:
  static char ID;

  std::vector<std::pair<Value *, Value *>> inst_edges; // 指令顺序边
  std::vector<std::pair<Value *, Value *>> edges; // data flow边
  std::vector<Value *> nodes; // 指令节点
  int num = 0;

  DFGPass() : FunctionPass(ID) {}

  // 如果是变量则获得变量的名字，如果是指令则获得SSA编号
  std::string getValueName(Value *v) {
    if (!v) {
      return "undefined";
    }
    std::string result;
    if (v->getName().empty()) {
      result = std::to_string(num++);
    } else {
      result = v->getName().str();
    }
    return result;
  }

  bool runOnFunction(Function &F) override {
    dbgs() << "DFGPass::runOnFunction: called on " << F.getName() << " @ "
           << F.getParent()->getName() << ".\n";

    std::error_code error_code;
    sys::fs::OpenFlags flags = sys::fs::OpenFlags::OF_Text;
    std::string fileName(F.getName().str() + ".dot");
    raw_fd_ostream file(fileName, error_code, flags);

    inst_edges.clear();
    edges.clear();
    nodes.clear();

    for (auto &BB : F) {
      for (auto I = BB.begin(), E = BB.end(); I != E; ++I) {
        Instruction *inst = &*I;

        // 去除bitwidth元数据
        if (inst->getMetadata("bitwidth")) {
          inst->setMetadata("bitwidth", nullptr);
        }

        switch (inst->getOpcode()) {
        case llvm::Instruction::Load: {
          LoadInst *loadInst = cast<LoadInst>(inst);
          Value *loadValPtr = loadInst->getPointerOperand();
          edges.push_back({loadValPtr, inst});
          break;
        }

        case llvm::Instruction::Store: {
          StoreInst *storeInst = dyn_cast<StoreInst>(inst);
          Value *storeValPtr = storeInst->getPointerOperand();
          Value *storeVal = storeInst->getValueOperand();
          edges.push_back({storeVal, inst});
          edges.push_back({inst, storeValPtr});
          break;
        }

        default: {
          for (auto &op : inst->operands()) {
            if (isa<Instruction>(op)) {
              edges.push_back({op.get(), inst});
            }
          }
          break;
        }
        }

        nodes.push_back(inst);
        auto nextI = std::next(I);
        if (nextI != E) {
          inst_edges.push_back({inst, &*nextI});
        }
      }

      Instruction *terminator = BB.getTerminator();
      for (auto sucBB : successors(&BB)) {
        Instruction *firstInst = &*(sucBB->begin());
        inst_edges.push_back({terminator, firstInst});
      }
    }

    dbgs() << "DFGPass::runOnFunction: dumping DFG to dot file.\n";
    file << "digraph \"DFG for '" + F.getName() + "\' function\" {\n";
    // dump node
    for (auto &node : nodes) {
      if (auto *inst = dyn_cast<Instruction>(node)) {
        std::string s;
        raw_string_ostream os(s);
        if (auto *callInst = dyn_cast<CallInst>(inst)) {
          os << (inst->getName().empty() ? "" : "%" + inst->getName() + " = ")
             << "call "
             << "@" << callInst->getCalledFunction()->getName();
        } else {
          os << *inst;
        }
        file << "\tNode" << node << "[shape=record, label=\"" << os.str()
             << "\"];\n";
      } else {
        file << "\tNode" << node << "[shape=record, label=\""
             << getValueName(node) << "\"];\n";
      }
    }

    // dump inst_edges
    for (auto &edge : inst_edges) {
      file << "\tNode" << edge.first << " -> Node" << edge.second << "\n";
    }

    // dump edges
    file << "edge [color=red]" << "\n";
    for (auto &edge : edges) {
      file << "\tNode" << edge.first << " -> Node" << edge.second << "\n";
    }

    file << "}\n";
    file.close();

    return false;
  }
};
} // namespace

char DFGPass::ID = 0;
static RegisterPass<DFGPass> X("DFGPass", "DFG Pass Analyse", false, false);
