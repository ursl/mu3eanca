# Shared MU3E checkout / build / relink rules.
#
# Required in the including Snakefile (before include):
#   MU3E_REPO, MU3E_DIR, MU3E_WORK_BASEDIR, MAKE_JOBS, RELINK_SCRIPT
#   MU3E_CHECKOUT_REF        tag/ref when not using a branch (empty if branch mode)
#   MU3E_CHECKOUT_BRANCH     branch name to track at origin/BRANCH HEAD (optional)
#   MU3E_CHECKOUT_MERGES     list of commit hashes to git merge after checkout (optional)
#   CLONE_MU3E_INPUTS       list with one prerequisite path (bootstrap or local marker)
#   CLONE_MU3E_SCRIPT       path to scripts/clone_and_prepare_mu3e.sh
#   MU3E_PREP_LOG_PREFIX    log tag, e.g. "relval" or "rereco"


rule clone_and_prepare_mu3e:
    input:
        CLONE_MU3E_INPUTS
    output:
        ".markers/clone_and_prepare_mu3e.done"
    params:
        repo=MU3E_REPO,
        ref=MU3E_CHECKOUT_REF,
        branch=MU3E_CHECKOUT_BRANCH,
        merge_list=" ".join(MU3E_CHECKOUT_MERGES),
        work_basedir=MU3E_WORK_BASEDIR,
        mu3e_dir=MU3E_DIR,
        script=CLONE_MU3E_SCRIPT
    shell:
        r"""
        set -euo pipefail
        args=(
            --repo "{params.repo}"
            --work-basedir "{params.work_basedir}"
            --mu3e-dir "{params.mu3e_dir}"
            --marker "{output}"
        )
        if [ -n "{params.branch}" ]; then
            args+=(--branch "{params.branch}")
        else
            args+=(--ref "{params.ref}")
        fi
        if [ -n "{params.merge_list}" ]; then
            read -r -a _merges <<< "{params.merge_list}"
            for _m in "${{_merges[@]}}"; do
                args+=(--merge "$_m")
            done
        fi
        bash "{params.script}" "${{args[@]}}"
        """


rule build_mu3e:
    input:
        ".markers/clone_and_prepare_mu3e.done"
    output:
        ".markers/build_mu3e.done"
    params:
        mu3e_dir=MU3E_DIR,
        marker_path=f"{MU3E_DIR}/.markers/build_mu3e.done",
        jobs=MAKE_JOBS
    shell:
        r"""
        set -euo pipefail
        mkdir -p "{params.mu3e_dir}/_build"
        cd "{params.mu3e_dir}/_build"
        cmake ..
        make -j{params.jobs}
        touch "{params.marker_path}"
        """


rule relink_bin_files:
    input:
        ".markers/build_mu3e.done"
    output:
        ".markers/relink_bin_files.done"
    params:
        mu3e_dir=MU3E_DIR,
        marker_path=f"{MU3E_DIR}/.markers/relink_bin_files.done",
        relink_script=RELINK_SCRIPT,
        log_prefix=MU3E_PREP_LOG_PREFIX
    shell:
        r"""
        set -euo pipefail
        cd "{params.mu3e_dir}/run"
        "{params.relink_script}"
        if [ ! -d "bvr2026" ]; then
            echo "[{params.log_prefix}] run/bvr2026 missing; fetching from bitbucket v6.5"
            tmpdir="$(mktemp -d)"
            git clone --depth 1 --branch v6.5 https://bitbucket.org/mu3e/mu3e "$tmpdir/mu3e-v6.5"
            if [ ! -d "$tmpdir/mu3e-v6.5/run/bvr2026" ]; then
                echo "[{params.log_prefix}] ERROR: fetched repo does not contain run/bvr2026" >&2
                rm -rf "$tmpdir"
                exit 1
            fi
            cp -R "$tmpdir/mu3e-v6.5/run/bvr2026" "./bvr2026"
            rm -rf "$tmpdir"
        fi
        mkdir -p output
        touch "{params.marker_path}"
        """
