# Clone and build mu3eUtil (expects checkout under MU3E via git submodules first).
#
# Required before include:
#   MU3E_WORK_BASEDIR, MU3E_UTIL_REPO, MU3E_UTIL_MAKE_JOBS, MU3E_UTIL_DIR
#   MU3E_UTIL_BOOTSTRAP_INPUTS   prerequisite rule outputs (e.g. clone_and_prepare_mu3e marker)


rule bootstrap_mu3e_util:
    input:
        MU3E_UTIL_BOOTSTRAP_INPUTS
    output:
        f"{MU3E_WORK_BASEDIR}/.bootstrap/mu3e_util.done"
    params:
        util_dir=MU3E_UTIL_DIR,
        jobs=MU3E_UTIL_MAKE_JOBS,
        log_prefix=MU3E_PREP_LOG_PREFIX
    shell:
        r"""
        set -euo pipefail
        mkdir -p "{MU3E_WORK_BASEDIR}/.bootstrap"

        if [ ! -f "{params.util_dir}/CMakeLists.txt" ]; then
            echo "[{params.log_prefix}] ERROR: mu3eUtil missing at {params.util_dir}" >&2
            echo "[{params.log_prefix}]        checkout mu3e first (clone_and_prepare_mu3e / git submodule update)" >&2
            exit 1
        fi
        mkdir -p "{params.util_dir}/_build"
        cd "{params.util_dir}/_build"
        cmake ..
        make -j{params.jobs}

        touch "{output}"
        echo "[{params.log_prefix}] mu3eUtil ready at {params.util_dir}/_build"
        """
