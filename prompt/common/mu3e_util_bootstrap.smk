# Clone and build mu3eUtil under MU3E_WORK_BASEDIR/mu3eUtil.
#
# Required before include:
#   MU3E_WORK_BASEDIR, MU3E_UTIL_REPO, MU3E_UTIL_MAKE_JOBS


rule bootstrap_mu3e_util:
    output:
        f"{MU3E_WORK_BASEDIR}/.bootstrap/mu3e_util.done"
    params:
        work_basedir=MU3E_WORK_BASEDIR,
        util_repo=MU3E_UTIL_REPO,
        jobs=MU3E_UTIL_MAKE_JOBS,
        log_prefix=MU3E_PREP_LOG_PREFIX
    shell:
        r"""
        set -euo pipefail
        mkdir -p "{params.work_basedir}"
        mkdir -p "{params.work_basedir}/.bootstrap"

        if [ ! -d "{params.work_basedir}/mu3eUtil/.git" ]; then
            git clone "{params.util_repo}" "{params.work_basedir}/mu3eUtil"
        fi
        mkdir -p "{params.work_basedir}/mu3eUtil/_build"
        cd "{params.work_basedir}/mu3eUtil/_build"
        cmake ..
        make -j{params.jobs}

        touch "{output}"
        echo "[{params.log_prefix}] mu3eUtil ready at {params.work_basedir}/mu3eUtil/_build"
        """
