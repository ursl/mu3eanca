# Run mu3e_midas_meta on one *.mid.lz4 file or on every *.mid.lz4 in a directory.
#
# Required before include:
#   MU3E_PREP_LOG_PREFIX, MIDAS_META_EXE, MIDAS_META_SCRIPT, MIDAS_META_MARKER_TPL
#   MIDAS_META_PREREQS          rule outputs required before midas_meta (e.g. build_mu3e marker)
#   midas_meta_input_dir(task), midas_meta_input_file(task), midas_meta_marker_path(task)
# Optional: MIDAS_META_FILE_SCRIPT (single-file mode)


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
        log_prefix=MU3E_PREP_LOG_PREFIX
    shell:
        r"""
        set -euo pipefail
        if [ -n "{params.input_file}" ]; then
            perl "{params.file_script}" \
                --exe "{params.exe}" \
                --input-file "{params.input_file}" \
                --marker "{params.marker_path}" \
                --log-prefix "{params.log_prefix}"
        else
            perl "{params.script}" \
                --exe "{params.exe}" \
                --input-dir "{params.input_dir}" \
                --marker "{params.marker_path}" \
                --log-prefix "{params.log_prefix}"
        fi
        """
