/**
 * @file lib.rs
 * @brief LLVM-based IR mutation engine for Heuristic OS.
 */

use inkwell::context::Context;
use inkwell::memory_buffer::MemoryBuffer;
use inkwell::module::Module;
use inkwell::targets::{InitializationConfig, Target, RelocMode, CodeModel, FileType};
use std::ptr;

extern "C" {
    static mut gen_buf: *mut u8;
    fn repo_write_api_add_variant(id: i32, size: i32);
}

/* Embedded LLVM bitcode of the worker logic */
static WORKER_BITCODE: &[u8] = include_bytes!("worker_logic.bc");

#[no_mangle]
pub extern "C" fn gen_api_request_generation(base_addr: u64, variant_id: i32, complexity: i32) {
    println!("[LLVM-GEN] INFO: Initializing LLVM dynamic synthesizer...");

    let context = Context::create();
    
    /* 1. Load reference bitcode from internal memory */
    let buffer = MemoryBuffer::create_from_memory_range(WORKER_BITCODE, "worker_core");
    let module = Module::parse_bitcode_from_buffer(&buffer, &context)
        .expect("[LLVM-GEN] FATAL: Failed to parse reference bitcode.");

    let builder = context.create_builder();
    let i32_type = context.i32_type();

    /* 2. Apply MTD Mutations (Dead Code Insertion) */
    println!("[LLVM-GEN] ACTION: Injecting mutations into IR-graph...");
    
    let mut mutations_applied = 0;
    for function in module.get_functions() {
        if function.count_basic_blocks() == 0 { continue; }

        for bb in function.get_basic_blocks() {
            if let Some(first_instr) = bb.get_first_instruction() {
                builder.position_before(&first_instr);
                
                /* Obfuscation: Insert computationally redundant logic */
                let seed_val = i32_type.const_int(variant_id as u64, false);
                let zero = i32_type.const_int(0, false);
                builder.build_int_add(seed_val, zero, "obf_junk").unwrap();
                
                mutations_applied += 1;
            }
        }
    }
    println!("[LLVM-GEN] INFO: {} mutations applied to CFG.", mutations_applied);

    /* 3. Native Code Synthesis (x86_64 PIC) */
    Target::initialize_x86(&InitializationConfig::default());
    let target = Target::from_triple(&"x86_64-unknown-linux-gnu".into()).unwrap();
    
    let machine = target.create_target_machine(
        &"x86_64-unknown-linux-gnu".into(),
        "generic",
        "",
        inkwell::OptimizationLevel::Default,
        RelocMode::PIC, /* Required for safe runtime injection */
        CodeModel::Small
    ).unwrap();

    let obj_buffer = machine.write_to_memory_buffer(&module, FileType::Object).unwrap();
    let obj_bytes = obj_buffer.as_slice();

    /* 4. Transfer to Repository via Dataport */
    unsafe {
        if !gen_buf.is_null() {
            let copy_size = obj_bytes.len().min(16777216); 
            ptr::copy_nonoverlapping(obj_bytes.as_ptr(), gen_buf, copy_size);
            
            println!("[LLVM-GEN] SUCCESS: Variant {} synthesized and transferred.", variant_id);
            repo_write_api_add_variant(variant_id, copy_size as i32);
        } else {
            eprintln!("[LLVM-GEN] ERROR: CAmkES dataport 'gen_buf' is NULL.");
        }
    }
}
