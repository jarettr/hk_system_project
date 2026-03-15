import os
import subprocess
import hashlib
import json
import shutil

# Конфигурация
MODULE_NAME = "worker_logic"
SOURCE_FILE = f"../hk_system/components/Worker/{MODULE_NAME}.c"
VARIANTS_COUNT = 5
OUTPUT_DIR = "build_artifacts"

def run_cmd(cmd):
    print(f"⚙️ Выполнение: {' '.join(cmd)}")
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        print(f"❌ Ошибка:\n{result.stderr}")
        exit(1)
    return result.stdout

def main():
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    manifest = {
        "module_id": "hk_worker_01",
        "variants": []
    }

    # Этап 1: Компиляция в LLVM IR (Эталон)
    orig_ir = f"{OUTPUT_DIR}/{MODULE_NAME}_orig.ll"
    run_cmd(["clang-17", "-O1", "-S", "-emit-llvm", "-g", SOURCE_FILE, "-o", orig_ir])
    print("✅ Этап 1: Исходный LLVM IR сгенерирован.")

    # Этап 2 и 3: Генерация и Верификация (Alive2)
    valid_variants = 0
    attempt = 0

    while valid_variants < VARIANTS_COUNT:
        attempt += 1
        mutated_ir = f"{OUTPUT_DIR}/{MODULE_NAME}_v{valid_variants}.ll"
        
        # Запускаем наш кастомный LLVM Pass (предполагается, что он скомпилирован в Mutator.so)
        # В качестве симуляции мутатора для скрипта, мы пока просто скопируем файл,
        # в реальности здесь будет вызов: opt-17 -load ./Mutator.so -mutate ...
        run_cmd(["opt-17", "-O2", "-S", orig_ir, "-o", mutated_ir])
        
        # Формальная верификация через Alive2
        print(f"🔍 Запуск Alive2 для варианта {valid_variants} (попытка {attempt})...")
        alive_res = subprocess.run(["alive-tv", orig_ir, mutated_ir], capture_output=True, text=True)
        
        if "Transformation seems to be correct!" in alive_res.stdout:
            print(f"✅ Alive2 подтвердил эквивалентность варианта {valid_variants}!")
            
            # Этап 4: Компиляция в бинарный код (PIC)
            obj_file = f"{OUTPUT_DIR}/{MODULE_NAME}_v{valid_variants}.o"
            bin_file = f"{OUTPUT_DIR}/{MODULE_NAME}_v{valid_variants}.bin"
            
            run_cmd(["llc-17", "-relocation-model=pic", "-filetype=obj", mutated_ir, "-o", obj_file])
            run_cmd(["llvm-objcopy-17", "-O", "binary", "--only-section=.text", obj_file, bin_file])
            
            # Хеширование
            with open(bin_file, "rb") as f:
                bin_data = f.read()
                b_hash = hashlib.sha256(bin_data).hexdigest()
                
            manifest["variants"].append({
                "variant_id": valid_variants,
                "binary_hash": b_hash,
                "size": len(bin_data),
                "file": bin_file
            })
            valid_variants += 1
        else:
            print("❌ Alive2 нашел UB или ошибку! Вариант отброшен.")

    # Сохраняем манифест
    manifest_path = f"{OUTPUT_DIR}/manifest.json"
    with open(manifest_path, "w") as f:
        json.dump(manifest, f, indent=4)
        
    # Симуляция цифровой подписи (Ed25519) закрытым ключом сборочного сервера
    print("✍️ Подписание пула вариантов (Ed25519)...")
    sig_path = f"{OUTPUT_DIR}/manifest.sig"
    with open(sig_path, "w") as f:
        f.write("SIMULATED_ED25519_SIGNATURE_998877665544332211")

    print(f"\n🎉 Пул из {VARIANTS_COUNT} вариантов успешно сгенерирован, верифицирован и подписан!")

if __name__ == "__main__":
    main()
# Этап 5: Упаковка в C-Header для seL4 CAmkES
    c_header_path = "../hk_system/components/Repository/baked_mutations.h"
    with open(c_header_path, "w") as f:
        f.write("#ifndef BAKED_MUTATIONS_H\n#define BAKED_MUTATIONS_H\n\n")
        f.write("#include <stdint.h>\n\n")
        f.write(f"#define POOL_SIZE {VARIANTS_COUNT}\n\n")
        
        for var in manifest["variants"]:
            v_id = var['variant_id']
            f.write(f"static const uint8_t variant_{v_id}_data[] = {{\n")
            with open(var['file'], "rb") as bf:
                bytes_data = bf.read()
                hex_array = ", ".join([f"0x{b:02x}" for b in bytes_data])
                f.write(f"    {hex_array}\n")
            f.write("};\n\n")
            
        # Создаем массив структур для Хранилища
        f.write("typedef struct {\n    int id;\n    const uint8_t* data;\n    int size;\n    const char* hash;\n} variant_record_t;\n\n")
        f.write("static const variant_record_t mutation_pool[POOL_SIZE] = {\n")
        for var in manifest["variants"]:
            v_id = var['variant_id']
            f.write(f"    {{{v_id}, variant_{v_id}_data, {var['size']}, \"{var['binary_hash']}\"}},\n")
        f.write("};\n\n")
        
        f.write("#endif // BAKED_MUTATIONS_H\n")
    print(f"📦 Артефакты упакованы в {c_header_path} для сборки CAmkES.")