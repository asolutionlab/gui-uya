#!/bin/bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
COMPILER="${UYA_COMPILER:-$REPO_ROOT/bin/uya}"
export UYA_ROOT="${REPO_ROOT}/lib/"

TMP_STDOUT="$(mktemp)"
TMP_STDERR="$(mktemp)"
trap 'rm -f "$TMP_STDOUT" "$TMP_STDERR"' EXIT

echo "验证显式类型局部 + catch 标识符路径..."
"$COMPILER" run --vm "$SCRIPT_DIR/test_exec_vm_compiler_typed_catch_local.uya" >"$TMP_STDOUT" 2>"$TMP_STDERR"
grep -q '后端类型: EXEC' "$TMP_STDERR"
grep -q 'exec backend 构建完成' "$TMP_STDERR"
grep -q 'typed catch local' "$TMP_STDOUT"
echo "  typed catch local ✓"

echo "验证 run --exec typed catch 路径不发生 fallback..."
"$COMPILER" run --exec "$SCRIPT_DIR/test_exec_vm_compiler_typed_catch_local.uya" >"$TMP_STDOUT" 2>"$TMP_STDERR"
grep -q '后端类型: EXEC' "$TMP_STDERR"
grep -q 'exec backend 构建完成' "$TMP_STDERR"
if grep -q '回退 C99' "$TMP_STDERR"; then
    echo "✗ typed catch local unexpectedly fell back to C99"
    cat "$TMP_STDERR"
    exit 1
fi
echo "  typed catch local --exec ✓"

echo "验证字段数组/指针字段/全局数组下标写入路径..."
"$COMPILER" run --vm "$SCRIPT_DIR/test_exec_vm_compiler_field_pointer_index.uya" >"$TMP_STDOUT" 2>"$TMP_STDERR"
grep -q '后端类型: EXEC' "$TMP_STDERR"
grep -q 'exec backend 构建完成' "$TMP_STDERR"
echo "  field/pointer/global index --vm ✓"

echo "验证 run --exec 字段数组/指针字段/全局数组下标写入路径不发生 fallback..."
"$COMPILER" run --exec "$SCRIPT_DIR/test_exec_vm_compiler_field_pointer_index.uya" >"$TMP_STDOUT" 2>"$TMP_STDERR"
grep -q '后端类型: EXEC' "$TMP_STDERR"
grep -q 'exec backend 构建完成' "$TMP_STDERR"
if grep -q '回退 C99' "$TMP_STDERR"; then
    echo "✗ field/pointer/global index unexpectedly fell back to C99"
    cat "$TMP_STDERR"
    exit 1
fi
echo "  field/pointer/global index --exec ✓"

echo "验证数组指针元素成员读取/写回路径..."
"$COMPILER" run --vm "$SCRIPT_DIR/test_exec_vm_compiler_array_ptr_member.uya" >"$TMP_STDOUT" 2>"$TMP_STDERR"
grep -q '后端类型: EXEC' "$TMP_STDERR"
grep -q 'exec backend 构建完成' "$TMP_STDERR"
echo "  array ptr member --vm ✓"

echo "验证 run --exec 下数组指针元素成员路径不发生 fallback..."
"$COMPILER" run --exec "$SCRIPT_DIR/test_exec_vm_compiler_array_ptr_member.uya" >"$TMP_STDOUT" 2>"$TMP_STDERR"
grep -q '后端类型: EXEC' "$TMP_STDERR"
grep -q 'exec backend 构建完成' "$TMP_STDERR"
if grep -q '回退 C99' "$TMP_STDERR"; then
    echo "✗ array ptr member unexpectedly fell back to C99"
    cat "$TMP_STDERR"
    exit 1
fi
echo "  array ptr member --exec ✓"

echo "验证 byte slice.ptr 读写与宿主桥接路径..."
"$COMPILER" run --vm "$SCRIPT_DIR/test_exec_vm_compiler_slice_ptr.uya" >"$TMP_STDOUT" 2>"$TMP_STDERR"
grep -q '后端类型: EXEC' "$TMP_STDERR"
grep -q 'exec backend 构建完成' "$TMP_STDERR"
echo "  slice.ptr --vm ✓"

echo "验证 run --exec 下 byte slice.ptr 路径不发生 fallback..."
"$COMPILER" run --exec "$SCRIPT_DIR/test_exec_vm_compiler_slice_ptr.uya" >"$TMP_STDOUT" 2>"$TMP_STDERR"
grep -q '后端类型: EXEC' "$TMP_STDERR"
grep -q 'exec backend 构建完成' "$TMP_STDERR"
if grep -q '回退 C99' "$TMP_STDERR"; then
    echo "✗ slice.ptr unexpectedly fell back to C99"
    cat "$TMP_STDERR"
    exit 1
fi
echo "  slice.ptr --exec ✓"

echo "验证跨文件 shared whole-module import 的 libc.stderr 成员访问..."
"$COMPILER" run --vm \
    "$SCRIPT_DIR/exec_vm_compiler_shared_libc_import/main.uya" \
    "$SCRIPT_DIR/exec_vm_compiler_shared_libc_import/shared_imports.uya" >"$TMP_STDOUT" 2>"$TMP_STDERR"
grep -q '后端类型: EXEC' "$TMP_STDERR"
grep -q 'exec backend 构建完成' "$TMP_STDERR"
grep -q 'shared libc import stderr' "$TMP_STDERR"
echo "  shared libc.stderr --vm ✓"

echo "验证 run --exec 下跨文件 shared whole-module import 的 libc.stderr 成员访问不发生 fallback..."
"$COMPILER" run --exec \
    "$SCRIPT_DIR/exec_vm_compiler_shared_libc_import/main.uya" \
    "$SCRIPT_DIR/exec_vm_compiler_shared_libc_import/shared_imports.uya" >"$TMP_STDOUT" 2>"$TMP_STDERR"
grep -q '后端类型: EXEC' "$TMP_STDERR"
grep -q 'exec backend 构建完成' "$TMP_STDERR"
grep -q 'shared libc import stderr' "$TMP_STDERR"
if grep -q '回退 C99' "$TMP_STDERR"; then
    echo "✗ shared libc.stderr unexpectedly fell back to C99"
    cat "$TMP_STDERR"
    exit 1
fi
echo "  shared libc.stderr --exec ✓"

echo "验证 error union .error_id 成员读取路径..."
"$COMPILER" run --vm "$SCRIPT_DIR/test_exec_vm_compiler_error_union_error_id.uya" >"$TMP_STDOUT" 2>"$TMP_STDERR"
grep -q '后端类型: EXEC' "$TMP_STDERR"
grep -q 'exec backend 构建完成' "$TMP_STDERR"
echo "  error union .error_id --vm ✓"

echo "验证 run --exec 下 error union .error_id 成员读取路径不发生 fallback..."
"$COMPILER" run --exec "$SCRIPT_DIR/test_exec_vm_compiler_error_union_error_id.uya" >"$TMP_STDOUT" 2>"$TMP_STDERR"
grep -q '后端类型: EXEC' "$TMP_STDERR"
grep -q 'exec backend 构建完成' "$TMP_STDERR"
if grep -q '回退 C99' "$TMP_STDERR"; then
    echo "✗ error union .error_id unexpectedly fell back to C99"
    cat "$TMP_STDERR"
    exit 1
fi
echo "  error union .error_id --exec ✓"

echo "验证 error union .value 成员读取路径..."
"$COMPILER" run --vm "$SCRIPT_DIR/test_exec_vm_compiler_error_union_value.uya" >"$TMP_STDOUT" 2>"$TMP_STDERR"
grep -q '后端类型: EXEC' "$TMP_STDERR"
grep -q 'exec backend 构建完成' "$TMP_STDERR"
echo "  error union .value --vm ✓"

echo "验证 run --exec 下 error union .value 成员路径不发生 fallback..."
"$COMPILER" run --exec "$SCRIPT_DIR/test_exec_vm_compiler_error_union_value.uya" >"$TMP_STDOUT" 2>"$TMP_STDERR"
grep -q '后端类型: EXEC' "$TMP_STDERR"
grep -q 'exec backend 构建完成' "$TMP_STDERR"
if grep -q '回退 C99' "$TMP_STDERR"; then
    echo "✗ error union .value unexpectedly fell back to C99"
    cat "$TMP_STDERR"
    exit 1
fi
echo "  error union .value --exec ✓"

echo "验证 union struct-field match 路径..."
"$COMPILER" run --vm "$SCRIPT_DIR/test_exec_vm_compiler_union_field_match.uya" >"$TMP_STDOUT" 2>"$TMP_STDERR"
grep -q '后端类型: EXEC' "$TMP_STDERR"
grep -q 'exec backend 构建完成' "$TMP_STDERR"
echo "  union field match --vm ✓"

echo "验证 run --exec 下 union struct-field match 路径不发生 fallback..."
"$COMPILER" run --exec "$SCRIPT_DIR/test_exec_vm_compiler_union_field_match.uya" >"$TMP_STDOUT" 2>"$TMP_STDERR"
grep -q '后端类型: EXEC' "$TMP_STDERR"
grep -q 'exec backend 构建完成' "$TMP_STDERR"
if grep -q '回退 C99' "$TMP_STDERR"; then
    echo "✗ union field match unexpectedly fell back to C99"
    cat "$TMP_STDERR"
    exit 1
fi
echo "  union field match --exec ✓"

echo "验证 match return block 中 union 结构体字段读取路径..."
"$COMPILER" run --vm "$SCRIPT_DIR/test_exec_vm_compiler_match_return_struct_field.uya" >"$TMP_STDOUT" 2>"$TMP_STDERR"
grep -q '后端类型: EXEC' "$TMP_STDERR"
grep -q 'exec backend 构建完成' "$TMP_STDERR"
echo "  match return struct field --vm ✓"

echo "验证 run --exec 下 match return block union 结构体字段路径不发生 fallback..."
"$COMPILER" run --exec "$SCRIPT_DIR/test_exec_vm_compiler_match_return_struct_field.uya" >"$TMP_STDOUT" 2>"$TMP_STDERR"
grep -q '后端类型: EXEC' "$TMP_STDERR"
grep -q 'exec backend 构建完成' "$TMP_STDERR"
if grep -q '回退 C99' "$TMP_STDERR"; then
    echo "✗ match return struct field unexpectedly fell back to C99"
    cat "$TMP_STDERR"
    exit 1
fi
echo "  match return struct field --exec ✓"

echo "验证 imported global 裸标识符读写路径..."
"$COMPILER" run --vm "$SCRIPT_DIR/test_exec_vm_compiler_imported_global_ident.uya" >"$TMP_STDOUT" 2>"$TMP_STDERR"
grep -q '后端类型: EXEC' "$TMP_STDERR"
grep -q 'exec backend 构建完成' "$TMP_STDERR"
echo "  imported global ident --vm ✓"

echo "验证 run --exec 下 imported global 裸标识符路径不发生 fallback..."
"$COMPILER" run --exec "$SCRIPT_DIR/test_exec_vm_compiler_imported_global_ident.uya" >"$TMP_STDOUT" 2>"$TMP_STDERR"
grep -q '后端类型: EXEC' "$TMP_STDERR"
grep -q 'exec backend 构建完成' "$TMP_STDERR"
if grep -q '回退 C99' "$TMP_STDERR"; then
    echo "✗ imported global ident unexpectedly fell back to C99"
    cat "$TMP_STDERR"
    exit 1
fi
echo "  imported global ident --exec ✓"

echo "验证全局数组元素取地址路径..."
"$COMPILER" run --vm "$SCRIPT_DIR/test_exec_vm_compiler_global_index_addr.uya" >"$TMP_STDOUT" 2>"$TMP_STDERR"
grep -q '后端类型: EXEC' "$TMP_STDERR"
grep -q 'exec backend 构建完成' "$TMP_STDERR"
echo "  global index addr --vm ✓"

echo "验证 run --exec 下全局数组元素取地址路径不发生 fallback..."
"$COMPILER" run --exec "$SCRIPT_DIR/test_exec_vm_compiler_global_index_addr.uya" >"$TMP_STDOUT" 2>"$TMP_STDERR"
grep -q '后端类型: EXEC' "$TMP_STDERR"
grep -q 'exec backend 构建完成' "$TMP_STDERR"
if grep -q '回退 C99' "$TMP_STDERR"; then
    echo "✗ global index addr unexpectedly fell back to C99"
    cat "$TMP_STDERR"
    exit 1
fi
echo "  global index addr --exec ✓"

echo "验证 runtime atomic global 直接读写与取址读取路径..."
"$COMPILER" run --vm "$SCRIPT_DIR/test_exec_vm_compiler_atomic_global.uya" >"$TMP_STDOUT" 2>"$TMP_STDERR"
grep -q '后端类型: EXEC' "$TMP_STDERR"
grep -q 'exec backend 构建完成' "$TMP_STDERR"
echo "  atomic global --vm ✓"

echo "验证 run --exec 下 runtime atomic global 路径不发生 fallback..."
"$COMPILER" run --exec "$SCRIPT_DIR/test_exec_vm_compiler_atomic_global.uya" >"$TMP_STDOUT" 2>"$TMP_STDERR"
grep -q '后端类型: EXEC' "$TMP_STDERR"
grep -q 'exec backend 构建完成' "$TMP_STDERR"
if grep -q '回退 C99' "$TMP_STDERR"; then
    echo "✗ atomic global unexpectedly fell back to C99"
    cat "$TMP_STDERR"
    exit 1
fi
echo "  atomic global --exec ✓"

echo "验证 repeat array literal 路径..."
"$COMPILER" run --vm "$SCRIPT_DIR/test_exec_vm_compiler_repeat_array_literal.uya" >"$TMP_STDOUT" 2>"$TMP_STDERR"
grep -q '后端类型: EXEC' "$TMP_STDERR"
grep -q 'exec backend 构建完成' "$TMP_STDERR"
echo "  repeat array literal --vm ✓"

echo "验证 run --exec 下 repeat array literal 路径不发生 fallback..."
"$COMPILER" run --exec "$SCRIPT_DIR/test_exec_vm_compiler_repeat_array_literal.uya" >"$TMP_STDOUT" 2>"$TMP_STDERR"
grep -q '后端类型: EXEC' "$TMP_STDERR"
grep -q 'exec backend 构建完成' "$TMP_STDERR"
if grep -q '回退 C99' "$TMP_STDERR"; then
    echo "✗ repeat array literal unexpectedly fell back to C99"
    cat "$TMP_STDERR"
    exit 1
fi
echo "  repeat array literal --exec ✓"

echo "验证空 struct init 零填充路径..."
"$COMPILER" run --vm "$SCRIPT_DIR/test_exec_vm_compiler_empty_struct_init.uya" >"$TMP_STDOUT" 2>"$TMP_STDERR"
grep -q '后端类型: EXEC' "$TMP_STDERR"
grep -q 'exec backend 构建完成' "$TMP_STDERR"
echo "  empty struct init --vm ✓"

echo "验证 run --exec 下空 struct init 路径不发生 fallback..."
"$COMPILER" run --exec "$SCRIPT_DIR/test_exec_vm_compiler_empty_struct_init.uya" >"$TMP_STDOUT" 2>"$TMP_STDERR"
grep -q '后端类型: EXEC' "$TMP_STDERR"
grep -q 'exec backend 构建完成' "$TMP_STDERR"
if grep -q '回退 C99' "$TMP_STDERR"; then
    echo "✗ empty struct init unexpectedly fell back to C99"
    cat "$TMP_STDERR"
    exit 1
fi
echo "  empty struct init --exec ✓"

echo "验证 @asm_target() as! i32 路径..."
"$COMPILER" run --vm "$SCRIPT_DIR/test_exec_vm_compiler_asm_target.uya" >"$TMP_STDOUT" 2>"$TMP_STDERR"
grep -q '后端类型: EXEC' "$TMP_STDERR"
grep -q 'exec backend 构建完成' "$TMP_STDERR"
echo "  asm_target --vm ✓"

echo "验证 run --exec 下 @asm_target() 路径不发生 fallback..."
"$COMPILER" run --exec "$SCRIPT_DIR/test_exec_vm_compiler_asm_target.uya" >"$TMP_STDOUT" 2>"$TMP_STDERR"
grep -q '后端类型: EXEC' "$TMP_STDERR"
grep -q 'exec backend 构建完成' "$TMP_STDERR"
if grep -q '回退 C99' "$TMP_STDERR"; then
    echo "✗ asm_target unexpectedly fell back to C99"
    cat "$TMP_STDERR"
    exit 1
fi
echo "  asm_target --exec ✓"

echo "验证 _ = expr; 丢弃赋值路径..."
"$COMPILER" run --vm "$SCRIPT_DIR/test_exec_vm_discard_assign.uya" >"$TMP_STDOUT" 2>"$TMP_STDERR"
grep -q '后端类型: EXEC' "$TMP_STDERR"
grep -q 'exec backend 构建完成' "$TMP_STDERR"
grep -q 'discard assign' "$TMP_STDOUT"
echo "  discard assign --vm ✓"

echo "验证 run --exec 下 _ = expr; 丢弃赋值路径不发生 fallback..."
"$COMPILER" run --exec "$SCRIPT_DIR/test_exec_vm_discard_assign.uya" >"$TMP_STDOUT" 2>"$TMP_STDERR"
grep -q '后端类型: EXEC' "$TMP_STDERR"
grep -q 'exec backend 构建完成' "$TMP_STDERR"
if grep -q '回退 C99' "$TMP_STDERR"; then
    echo "✗ discard assign unexpectedly fell back to C99"
    cat "$TMP_STDERR"
    exit 1
fi
echo "  discard assign --exec ✓"

echo "验证 catch 前缀副作用 block 路径..."
"$COMPILER" run --vm "$SCRIPT_DIR/test_exec_vm_catch_block_prefix.uya" >"$TMP_STDOUT" 2>"$TMP_STDERR"
grep -q '后端类型: EXEC' "$TMP_STDERR"
grep -q 'exec backend 构建完成' "$TMP_STDERR"
grep -q 'catch prefix return' "$TMP_STDOUT"
echo "  catch prefix block --vm ✓"

echo "验证 run --exec 下 catch 前缀副作用 block 路径不发生 fallback..."
"$COMPILER" run --exec "$SCRIPT_DIR/test_exec_vm_catch_block_prefix.uya" >"$TMP_STDOUT" 2>"$TMP_STDERR"
grep -q '后端类型: EXEC' "$TMP_STDERR"
grep -q 'exec backend 构建完成' "$TMP_STDERR"
if grep -q '回退 C99' "$TMP_STDERR"; then
    echo "✗ catch prefix block unexpectedly fell back to C99"
    cat "$TMP_STDERR"
    exit 1
fi
echo "  catch prefix block --exec ✓"

echo "验证 struct sizeof/alignof 在 --vm 下直接折叠..."
"$COMPILER" run --vm "$SCRIPT_DIR/test_exec_vm_compiler_sizeof_struct.uya" >"$TMP_STDOUT" 2>"$TMP_STDERR"
grep -q '后端类型: EXEC' "$TMP_STDERR"
grep -q 'exec backend 构建完成' "$TMP_STDERR"
echo "  sizeof/alignof struct --vm ✓"

echo "验证 run --exec struct sizeof/alignof 路径不发生 fallback..."
"$COMPILER" run --exec "$SCRIPT_DIR/test_exec_vm_compiler_sizeof_struct.uya" >"$TMP_STDOUT" 2>"$TMP_STDERR"
grep -q '后端类型: EXEC' "$TMP_STDERR"
grep -q 'exec backend 构建完成' "$TMP_STDERR"
if grep -q '回退 C99' "$TMP_STDERR"; then
    echo "✗ sizeof/alignof struct unexpectedly fell back to C99"
    cat "$TMP_STDERR"
    exit 1
fi
echo "  sizeof/alignof struct --exec ✓"

echo "✓ exec vm compiler regression checks passed"
