use inkwell::context::Context;
use inkwell::memory_buffer::MemoryBuffer;
use inkwell::module::Module;
use inkwell::targets::{InitializationConfig, Target, RelocMode, CodeModel, FileType};
use std::ptr;

// Импортируем порты и функции CAmkES
extern "C" {
    static mut gen_buf: *mut u8; // 16MB буфер из .camkes
    fn repo_write_add_variant(id: i32, threat: i32);
}

// Запекаем эталонный IR прямо в бинарник (надежно и без файловой системы!)
static WORKER_BITCODE: &[u8] = include_bytes!("worker_logic.bc");

#[no_mangle]
pub extern "C" fn gen_api_request_generation(threat_type: i32) {
    println!("\n[LLVM-PARSER] ⚙️ Инициализация боевого парсера LLVM IR...");

    let context = Context::create();
    
    // 1. ЗАГРУЗКА ЭТАЛОНА ИЗ ПАМЯТИ
    let buffer = MemoryBuffer::create_from_memory_range(WORKER_BITCODE, "worker_logic");
    let module = Module::parse_bitcode_from_buffer(&buffer, &context)
        .expect("[LLVM-PARSER] ❌ Ошибка парсинга .bc файла!");

    let builder = context.create_builder();
    let i32_type = context.i32_type();

    // 2. ПРИМЕНЕНИЕ МУТАЦИЙ (ТЗ 4.2)
    println!("[LLVM-PARSER] 🧬 Сканирование IR-графа и применение мутаций...");
    
    let mut mutation_count = 0;
    for function in module.get_functions() {
        // Пропускаем внешние функции (например, системные вызовы)
        if function.count_basic_blocks() == 0 { continue; }

        for bb in function.get_basic_blocks() {
            // Берем первую инструкцию в базовом блоке
            if let Some(first_instr) = bb.get_first_instruction() {
                builder.position_before(&first_instr);
                
                // Вставляем мертвый код (Dead Code Insertion). 
                // Для декомпилятора хакера это выглядит как реальная логика.
                // dummy_var = threat_type + 0;
                let dummy_val = i32_type.const_int(threat_type as u64, false);
                let zero = i32_type.const_int(0, false);
                builder.build_int_add(dummy_val, zero, "mut_junk").unwrap();
                
                mutation_count += 1;
            }
        }
    }
    println!("[LLVM-PARSER] Внедрено {} мутаций на уровне базовых блоков.", mutation_count);

    // 3. КОМПИЛЯЦИЯ В МАШИННЫЙ КОД (x86_64)
    Target::initialize_x86(&InitializationConfig::default());
    let target = Target::from_triple(&"x86_64-unknown-linux-gnu".into()).unwrap();
    
    // КРИТИЧНО: RelocMode::PIC позволяет Диспетчеру грузить код по любому адресу!
    let machine = target.create_target_machine(
        &"x86_64-unknown-linux-gnu".into(),
        "generic",
        "",
        inkwell::OptimizationLevel::Default,
        RelocMode::PIC, 
        CodeModel::Small
    ).unwrap();

    println!("[LLVM-PARSER] 🔨 Генерация объектного PIC-кода...");
    let obj_buffer = machine.write_to_memory_buffer(&module, FileType::Object).unwrap();
    let obj_bytes = obj_buffer.as_slice();

    // 4. ПЕРЕДАЧА В ХРАНИЛИЩЕ ЧЕРЕЗ DATAPORT
    unsafe {
        if !gen_buf.is_null() {
            // Защита от переполнения буфера (gen_buf у нас 16 МБ)
            let copy_size = obj_bytes.len().min(16777216); 
            ptr::copy_nonoverlapping(obj_bytes.as_ptr(), gen_buf, copy_size);
            
            println!("[LLVM-PARSER] ✅ Скомпилированный бинарник ({} байт) загружен в gen_buf.", copy_size);
            
            // Сообщаем Хранилищу, что данные готовы (генерим уникальный ID)
            static mut NEXT_ID: i32 = 1000;
            repo_write_add_variant(NEXT_ID, threat_type);
            NEXT_ID += 1;
        } else {
            println!("[LLVM-PARSER] ❌ КРИТИЧЕСКАЯ ОШИБКА: gen_buf не инициализирован!");
        }
    }
}