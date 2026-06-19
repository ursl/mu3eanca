# Shared mu3eTrirec rule.
#
# Required in the including Snakefile (define before include):
#   MU3E_DIR, MU3E_PREP_LOG_PREFIX, CDB_DBCONN, CDB_GT
#   TRIREC_OUTPUT_TPL          e.g. output/trirec-{scenario}.root or ...-{job}.root
#   TRIREC_LOG_TPL             e.g. logs/snakemake/run_mu3e_trirec-{scenario}.log
#   trirec_rule_inputs(wc)     -> list of input paths (sort only, or relink + sort)
#   trirec_sort_input(wc)      -> sort ROOT path passed to mu3eTrirec
#   trirec_output_rel(wc)      -> output path relative to run/
#   trirec_conf_for(wc)        -> trirec conf filename (under run/)
#   trirec_conf_fallback_for(wc)
#   trirec_run_id_for(wc)      -> run number for --run
#   trirec_item_label(wc)      -> wildcard value for log lines (scenario id or job id)


rule run_mu3e_trirec:
    input:
        lambda wc: trirec_rule_inputs(wc),
    output:
        f"{MU3E_DIR}/run/{TRIREC_OUTPUT_TPL}"
    log:
        TRIREC_LOG_TPL
    params:
        mu3e_dir=MU3E_DIR,
        sort_input=lambda wc: trirec_sort_input(wc),
        trirec_output=lambda wc: trirec_output_rel(wc),
        trirec_conf=lambda wc: trirec_conf_for(wc),
        trirec_conf_fallback=lambda wc: trirec_conf_fallback_for(wc),
        run_id=lambda wc: trirec_run_id_for(wc),
        item_label=lambda wc: trirec_item_label(wc),
        cdb_dbconn=CDB_DBCONN,
        cdb_GT=CDB_GT,
        log_prefix=MU3E_PREP_LOG_PREFIX
    shell:
        r"""
        set -euo pipefail
        cd "{params.mu3e_dir}/run"
        if [ ! -f "{params.sort_input}" ]; then
            echo "[{params.log_prefix}] ERROR: sort input missing: {params.sort_input}" >&2
            exit 1
        fi
        trirec_conf="{params.trirec_conf}"
        if [ ! -f "$trirec_conf" ]; then
            if [ -n "{params.trirec_conf_fallback}" ] && [ -f "{params.trirec_conf_fallback}" ]; then
                echo "[{params.log_prefix}] trirec conf '$trirec_conf' missing, using fallback '{params.trirec_conf_fallback}'"
                trirec_conf="{params.trirec_conf_fallback}"
            else
                echo "[{params.log_prefix}] ERROR: trirec conf '$trirec_conf' not found and no valid fallback configured" >&2
                exit 1
            fi
        fi
        echo "[{params.log_prefix}] item={params.item_label} sort={params.sort_input} conf=$trirec_conf run={params.run_id}"
        ../_build/mu3eTrirec/mu3eTrirec \
            "{params.sort_input}" \
            --conf "$trirec_conf" \
            --output "{params.trirec_output}" \
            --run {params.run_id} \
            --cdb.dbconn="{params.cdb_dbconn}" \
            --cdb.globalTag="{params.cdb_GT}"
        """
