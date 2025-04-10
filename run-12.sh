if [ -z "$1" ]; then
  echo "Usage: $0 <input.bc>"
  exit 1
fi

llvm-dis-12 "./test/$1" -o "./test/a.o.3.ll"
sed -i '/target triple = "fpga64-xilinx-none"/d' "./test/a.o.3.ll"

if [ -d "./test/output" ]; then
  rm -rf "./test/output"
fi
mkdir -p "./test/output"

cd "./test/output"
opt-12 \
  -enable-new-pm=false \
  -load "../../build/DFGPass/libLLVMDFGPass.so" \
  -strip-debug \
  -DFGPass \
  "../../test/a.o.3.ll"
python3 "../convert.py" "../../test/output"
