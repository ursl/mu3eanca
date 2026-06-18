# Run mu3e_midas_meta on one *.mid.lz4 file or on every *.mid.lz4 in a directory.
#
# Required before include:
#   MU3E_PREP_LOG_PREFIX, MIDAS_META_EXE, MIDAS_META_SCRIPT, MIDAS_META_MARKER_TPL
#   MIDAS_META_PREREQS          rule outputs required before midas_meta (e.g. build_mu3e marker)
#   midas_meta_input_dir(task), midas_meta_input_file(task), midas_meta_marker_path(task)
# Optional: MIDAS_META_FILE_SCRIPT (single-file mode)
# Optional submit (defaults: local via run_midas_meta_submit):
#   JOB_SUBMIT_MODE, MIDAS_META_SUBMIT_SCRIPT, SETUP_DIR, SETUP_ROOT
#   SLURM_PARTITION, SLURM_LOG_DIR, SLURM_BATCH_SCRIPT
#   midas_meta_task_run_id(task), midas_meta_task_label(task)


rule run_midas_meta:
    input:
        MIDAS_META_PREREQS
    output:
        MIDAS_META_MARKER_TPL
    log:
        "logs/snakemake/run_midas_meta-{task}.log"
    params:
        input_dir=lambda wc: midas_meta_input_dir(wc.task),
        input_file=lambda wc: midas_meta_input_file(wc.task),
        marker_path=lambda wc: midas_meta_marker_path(wc.task),
        exe=MIDAS_META_EXE,
        script=MIDAS_META_SCRIPT,
        file_script=lambda wc: MIDAS_META_FILE_SCRIPT,
        log_prefix=MU3E_PREP_LOG_PREFIX,
        submit_mode=JOB_SUBMIT_MODE,
        submit_script=MIDAS_META_SUBMIT_SCRIPT,
        rundir=SETUP_ROOT,
        setup_dir=SETUP_DIR,
        task_run=lambda wc: midas_meta_task_run_id(wc.task),
        task_label=lambda wc: midas_meta_task_label(wc.task),
        slurm_partition=SLURM_PARTITION,
        slurm_logdir=SLURM_LOG_DIR,
        slurm_script=SLURM_BATCH_SCRIPT,
    shell:
        r"""
        set -euo pipefail
        if [ -n "{params.input_file}" ]; then
            perl "{params.submit_script}" \
                --mode "{params.submit_mode}" \
                --exe "{params.exe}" \
                --input-file "{params.input_file}" \
                --marker "{params.marker_path}" \
                --log-prefix "{params.log_prefix}" \
                --rundir "{params.rundir}" \
                --setup "{params.setup_dir}" \
                --task "{params.task_label}" \
                --run "{params.task_run}" \
                --slurm-partition "{params.slurm_partition}" \
                --slurm-logdir "{params.slurm_logdir}" \
                --slurm-script "{params.slurm_script}" \
                --file-script "{params.file_script}" \
                --dir-script "{params.script}"
        else
            perl "{params.submit_script}" \
                --mode local \
                --exe "{params.exe}" \
                --input-dir "{params.input_dir}" \
                --marker "{params.marker_path}" \
                --log-prefix "{params.log_prefix}" \
                --file-script "{params.file_script}" \
                --dir-script "{params.script}"
        fi
        """
