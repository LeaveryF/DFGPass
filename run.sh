llvm-dis-14 "./test/a.o.3.bc" -o "./test/a.o.3.ll"
sed -i '/target triple = "fpga64-xilinx-none"/d' "./test/a.o.3.ll"

if [ -d "./test/output" ]; then
  rm -rf "./test/output"
fi
mkdir -p "./test/output"

cd "./test/output"
opt-14 \
  -enable-new-pm=false \
  -load "../../build/DFGPass/libLLVMDFGPass.so" \
  -strip \
  -DFGPass \
  "../../test/a.o.3.ll"
python3 "../convert.py" "../../test/output"
