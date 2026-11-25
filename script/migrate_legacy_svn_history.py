#!/usr/bin/env python3
"""
migrate_legacy_svn_history.py:
Tool to map Subversion release tags to Git commits and reconstruct release branches
(0.1.x, 0.2.x, 0.3.x, 0.4.x) and legacy release tags (legacy_<tag>).

Uses Git plumbing (git write-tree, git commit-tree, git update-ref, git tag)
to create branches and tags without modifying the active working tree or index.
"""

import sys
import os
import subprocess
import argparse
import tempfile

# Specification of Branches, Steps, and Legacy Tags
MIGRATION_PLAN = [
    {
        "branch": "0.1.x",
        "description": "Early CVS/SVN Milestones & 0.1.x Releases",
        "steps": [
            {
                "tag": "legacy_betareleasecandidate102901",
                "base_commit": "b6dad1de1",
                "svn_tag_dir": "betareleasecandidate102901",
                "author": "Daniel Horn <hellcatv@hotmail.com>",
                "date": "2001-10-29T10:09:50Z",
                "replay_diffs": False
            },
            {
                "tag": "legacy_vegastrike_0.1.0",
                "base_commit": "5982d73d5",
                "svn_tag_dir": None,
                "author": "Daniel Horn <hellcatv@hotmail.com>",
                "date": "2001-11-07T03:19:24Z",
                "replay_diffs": False
            },
            {
                "tag": "legacy_vegastrike_0.1.3",
                "base_commit": "11a75d15f",
                "svn_tag_dir": None,
                "author": "Daniel Horn <hellcatv@hotmail.com>",
                "date": "2002-01-15T19:58:58Z",
                "replay_diffs": False
            },
            {
                "tag": "legacy_after_data_removal_stage1",
                "base_commit": "953a80b5e",
                "svn_tag_dir": "after_data_removal_stage1",
                "author": "Alexander Rawass <alexannika@users.sourceforge.net>",
                "date": "2002-01-17T08:45:25Z",
                "replay_diffs": False
            },
            {
                "tag": "legacy_vegastrike_snapshot_020117",
                "base_commit": "da2f1f573",
                "svn_tag_dir": "vegastrike_snapshot_020117_data",
                "author": "Alexander Rawass <alexannika@users.sourceforge.net>",
                "date": "2002-01-18T12:44:24Z",
                "replay_diffs": False
            }
        ]
    },
    {
        "branch": "0.2.x",
        "description": "0.2 Release Line (0.2.0 to 0.2.9.4)",
        "steps": [
            {
                "tag": "legacy_release-0-2-0",
                "base_commit": "4851b9a474",
                "svn_tag_dir": "release-0-2-0",
                "author": "Daniel Horn <hellcatv@hotmail.com>",
                "date": "2002-02-19T20:43:33Z",
                "replay_diffs": False
            },
            {
                "tag": "legacy_vegastrike_0_2_1",
                "base_commit": "d85e6b370b",
                "svn_tag_dir": "vegastrike_0_2_1",
                "author": "Daniel Horn <hellcatv@hotmail.com>",
                "date": "2002-03-11T09:58:39Z",
                "replay_diffs": False
            },
            {
                "tag": "legacy_stable_050102",
                "base_commit": "5508a52ced",
                "svn_tag_dir": "stable_050102",
                "author": "Daniel Horn <hellcatv@hotmail.com>",
                "date": "2002-05-01T10:43:04Z",
                "replay_diffs": False
            },
            {
                "tag": "legacy_vegastrike_0_2_9",
                "base_commit": "991286e8b3",
                "svn_tag_dir": "vegastrike_0_2_9",
                "author": "Daniel Horn <hellcatv@hotmail.com>",
                "date": "2002-06-27T06:10:23Z",
                "replay_diffs": True
            },
            {
                "tag": "legacy_vegastrike_0_2_9_2",
                "base_commit": "f9994a9093",
                "svn_tag_dir": "vegastrike_0_2_9_2",
                "author": "Daniel Horn <hellcatv@hotmail.com>",
                "date": "2002-07-01T06:21:15Z",
                "replay_diffs": True
            },
            {
                "tag": "legacy_vegastrike_0_2_9_3",
                "base_commit": "31bf422c2d",
                "svn_tag_dir": "vegastrike_0_2_9_3",
                "author": "Daniel Horn <hellcatv@hotmail.com>",
                "date": "2002-08-30T23:53:03Z",
                "replay_diffs": True
            },
            {
                "tag": "legacy_vegastrike_0_2_9_4",
                "base_commit": "ec47a2dae6",
                "svn_tag_dir": "vegastrike_0_2_9_4",
                "author": "Daniel Horn <hellcatv@hotmail.com>",
                "date": "2002-08-30T23:53:03Z",
                "replay_diffs": True
            }
        ]
    },
    {
        "branch": "0.3.x",
        "description": "0.3 Release Line (0.3.0 to 0.3.9)",
        "steps": [
            {
                "tag": "legacy_vegastrike_0_3_0",
                "base_commit": "7f3d3e9605",
                "svn_tag_dir": "vegastrike_0_3_0",
                "author": "Daniel Horn <hellcatv@hotmail.com>",
                "date": "2003-01-07T07:27:12Z",
                "replay_diffs": False
            },
            {
                "tag": "legacy_vegastrike_0_3_1",
                "base_commit": "260d8683d3",
                "svn_tag_dir": "vegastrike_0_3_1",
                "author": "Daniel Horn <hellcatv@hotmail.com>",
                "date": "2003-01-10T03:12:56Z",
                "replay_diffs": False
            },
            {
                "tag": "legacy_vegastrike_0_3_3",
                "base_commit": "00a86aae21",
                "svn_tag_dir": "vegastrike_0_3_3",
                "author": "Daniel Horn <hellcatv@hotmail.com>",
                "date": "2003-01-24T12:52:57Z",
                "replay_diffs": False
            },
            {
                "tag": "legacy_vegastrike_0_3_9",
                "base_commit": "30a8218763",
                "svn_tag_dir": "vegastrike_0_3_9",
                "author": "Daniel Horn <hellcatv@hotmail.com>",
                "date": "2003-06-30T06:03:48Z",
                "replay_diffs": True
            }
        ]
    },
    {
        "branch": "0.4.x",
        "description": "0.4 Release Line (0.4.0 to 0.4.3)",
        "steps": [
            {
                "tag": "legacy_vegastrike_0_4_0",
                "base_commit": "e961b81787",
                "svn_tag_dir": "vegastrike_0_4_0",
                "author": "Daniel Horn <hellcatv@hotmail.com>",
                "date": "2003-10-01T03:29:51Z",
                "replay_diffs": True
            },
            {
                "tag": "legacy_vegastrike_0_4_1",
                "base_commit": "926273a67a",
                "svn_tag_dir": "vegastrike_0_4_1",
                "author": "Daniel Horn <hellcatv@hotmail.com>",
                "date": "2003-10-24T03:06:41Z",
                "replay_diffs": True
            },
            {
                "tag": "legacy_vegastrike_0_4_2",
                "base_commit": "4633978af3",
                "svn_tag_dir": "vegastrike_0_4_2",
                "author": "Daniel Horn <hellcatv@hotmail.com>",
                "date": "2004-09-29T02:01:25Z",
                "replay_diffs": True
            },
            {
                "tag": "legacy_vegastrike_0_4_3",
                "base_commit": "da0f41011b",
                "svn_tag_dir": "vegastrike_0_4_3",
                "author": "Daniel Horn <hellcatv@hotmail.com>",
                "date": "2005-02-12T23:31:42Z",
                "replay_diffs": True
            }
        ]
    },
    {
        "branch": None,
        "description": "0.5.x Legacy Tags",
        "steps": [
            {
                "tag": "legacy_0.5.0",
                "base_commit": "103ce99fb",
                "svn_tag_dir": "0.5.0",
                "author": "ace123 <ace123@users.sourceforge.net>",
                "date": "2008-05-08T23:13:36Z",
                "replay_diffs": True
            },
            {
                "tag": "legacy_vegastrike_0_5_1_beta1",
                "base_commit": "0831baabb",
                "svn_tag_dir": "vegastrike_0_5_1_beta1",
                "author": "klaussfreire <klaussfreire@users.sourceforge.net>",
                "date": "2011-03-09T01:52:19Z",
                "replay_diffs": False
            },
            {
                "tag": "legacy_vegastrike_0_5_1_beta2",
                "base_commit": "4017c5bf9",
                "svn_tag_dir": "vegastrike_0_5_1_beta2",
                "author": "klaussfreire <klaussfreire@users.sourceforge.net>",
                "date": "2012-01-30T05:09:59Z",
                "replay_diffs": False
            },
            {
                "tag": "legacy_vegastrike_0_5_1_r1",
                "base_commit": "e24468df2",
                "svn_tag_dir": "vegastrike_0_5_1_r1",
                "author": "klaussfreire <klaussfreire@users.sourceforge.net>",
                "date": "2012-04-01T00:56:09Z",
                "replay_diffs": False
            },
            {
                "tag": "legacy_0.5.2alpha-archive",
                "base_commit": "752c33e46",
                "svn_tag_dir": "0.5.2alpha-archive",
                "author": "pyramid3d <pyramid@openteq.lan>",
                "date": "2020-02-06T14:18:57Z",
                "replay_diffs": False
            }
        ]
    }
]

def run_cmd(cmd, git_dir, env=None):
    res = subprocess.run(cmd, cwd=git_dir, env=env, capture_output=True, text=True)
    if res.returncode != 0:
        raise RuntimeError(f"Command failed ({res.returncode}): {' '.join(cmd)}\n{res.stderr}")
    return res.stdout.strip()

def get_full_commit(short_rev, git_dir):
    return run_cmd(["git", "rev-parse", short_rev], git_dir)

def create_tree_from_svn(svn_tags_dir, svn_tag_subpath, git_dir, temp_index):
    full_path = os.path.join(svn_tags_dir, svn_tag_subpath, "vegastrike")
    if not os.path.isdir(full_path):
        full_path = os.path.join(svn_tags_dir, svn_tag_subpath)
    
    if os.path.exists(temp_index):
        os.remove(temp_index)
        
    env = os.environ.copy()
    env["GIT_INDEX_FILE"] = temp_index
    env["GIT_DIR"] = os.path.join(git_dir, ".git")
    env["GIT_WORK_TREE"] = full_path
    
    run_cmd(["git", "add", "-A"], git_dir, env=env)
    tree_sha = run_cmd(["git", "write-tree"], git_dir, env=env)
    
    if os.path.exists(temp_index):
        os.remove(temp_index)
        
    return tree_sha

def resolve_svn_tags_dir(svn_path):
    """
    Resolve the Subversion tags directory from either an SVN root directory
    (which contains a 'tags' subdirectory) or a direct path to 'tags'.
    """
    if not svn_path:
        return None
    abs_path = os.path.abspath(os.path.expanduser(svn_path))
    # If the user passed the SVN root containing a 'tags' subdirectory
    tags_subdir = os.path.join(abs_path, "tags")
    if os.path.isdir(tags_subdir):
        return tags_subdir
    # If it's already the tags directory
    if os.path.isdir(abs_path):
        return abs_path
    return None

def find_default_svn_dir(repo_dir):
    """
    Discover default SVN location from environment variable or standard relative paths.
    """
    env_dir = os.environ.get("VEGA_SVN_DIR") or os.environ.get("SVN_DIR")
    if env_dir:
        resolved = resolve_svn_tags_dir(env_dir)
        if resolved:
            return resolved

    candidates = [
        os.path.join(repo_dir, "..", "vegastrike-code"),
        os.path.join(repo_dir, "..", "research", "vegastrike-code"),
        os.path.join(repo_dir, "..", "..", "research", "vegastrike-code"),
    ]
    for cand in candidates:
        resolved = resolve_svn_tags_dir(cand)
        if resolved:
            return resolved
    return None

def execute(git_dir, svn_source_dir, temp_index, dry_run=True):
    svn_tags_dir = resolve_svn_tags_dir(svn_source_dir)
    if not svn_tags_dir or not os.path.isdir(svn_tags_dir):
        print(f"Error: Subversion tags directory could not be resolved from: {svn_source_dir}", file=sys.stderr)
        print("Please specify a valid SVN repository root (containing 'tags/') or 'tags/' directory directly", file=sys.stderr)
        print("via --svn-dir <path> or the VEGA_SVN_DIR environment variable.", file=sys.stderr)
        print("Example: python3 script/migrate_legacy_svn_history.py --svn-dir /path/to/vegastrike-code", file=sys.stderr)
        sys.exit(1)

    print(f"=== Vega Strike Git Legacy Migration (Dry Run: {dry_run}) ===\n")
    print(f"  Git Repo: {git_dir}")
    print(f"  SVN Tags: {svn_tags_dir}\n")
    
    for group in MIGRATION_PLAN:
        branch = group["branch"]
        desc = group["description"]
        print(f"==================================================")
        print(f"Processing Series: {desc} (Branch: {branch or 'None'})")
        print(f"==================================================")
        
        current_branch_tip = None
        
        for step in group["steps"]:
            tag_name = step["tag"]
            base_rev = step["base_commit"]
            base_commit = get_full_commit(base_rev, git_dir)
            replay = step["replay_diffs"]
            svn_dir = step["svn_tag_dir"]
            author = step["author"]
            date_str = step["date"]
            
            print(f"\n--- Tag: {tag_name} ---")
            print(f"  Trunk Anchor: {base_commit[:10]} ({base_rev})")
            
            if replay and svn_dir:
                tag_path = os.path.join(svn_tags_dir, svn_dir)
                if not os.path.exists(tag_path):
                    raise FileNotFoundError(f"Subversion tag path does not exist: {tag_path}")
                print(f"  Reading SVN tree from: tags/{svn_dir}/vegastrike")
                tree_sha = create_tree_from_svn(svn_tags_dir, svn_dir, git_dir, temp_index)
                print(f"  Generated Tree SHA: {tree_sha[:10]}")
                
                base_tree = run_cmd(["git", "rev-parse", f"{base_commit}^{{tree}}"], git_dir)
                parent_commit = current_branch_tip if (branch and current_branch_tip) else base_commit
                
                if tree_sha != base_tree or (branch and current_branch_tip):
                    msg = f"Release {tag_name.replace('legacy_', '')} SVN tag tree\n\nMirrored from Subversion tags/{svn_dir}/vegastrike."
                    
                    env = os.environ.copy()
                    name, email = author.split("<")
                    name = name.strip()
                    email = email.replace(">", "").strip()
                    env["GIT_AUTHOR_NAME"] = name
                    env["GIT_AUTHOR_EMAIL"] = email
                    env["GIT_AUTHOR_DATE"] = date_str
                    env["GIT_COMMITTER_NAME"] = name
                    env["GIT_COMMITTER_EMAIL"] = email
                    env["GIT_COMMITTER_DATE"] = date_str
                    
                    cmd = ["git", "commit-tree", tree_sha, "-p", parent_commit, "-m", msg]
                    if dry_run:
                        sim_id = f"sim_{tag_name}"
                        print(f"  [DRY RUN] Would create commit with tree {tree_sha[:10]} and parent {parent_commit[:10]}")
                        commit_to_tag = sim_id
                    else:
                        new_commit = run_cmd(cmd, git_dir, env=env)
                        print(f"  Created Branch Commit: {new_commit[:10]} (parent: {parent_commit[:10]})")
                        commit_to_tag = new_commit
                else:
                    print(f"  Tree is identical to trunk commit {base_commit[:10]}; no extra commit needed.")
                    commit_to_tag = base_commit
            else:
                commit_to_tag = base_commit
                
            current_branch_tip = commit_to_tag
            
            # Tagging
            tag_msg = f"Legacy Subversion release tag: {tag_name}\n\nMirrored from SVN tag {svn_dir if svn_dir else tag_name}."
            tag_env = os.environ.copy()
            name, email = author.split("<")
            name = name.strip()
            email = email.replace(">", "").strip()
            tag_env["GIT_COMMITTER_NAME"] = name
            tag_env["GIT_COMMITTER_EMAIL"] = email
            tag_env["GIT_COMMITTER_DATE"] = date_str

            if dry_run:
                print(f"  [DRY RUN] Would create tag '{tag_name}' at {commit_to_tag[:10]}")
            else:
                run_cmd(["git", "tag", "-a", "-f", "-m", tag_msg, "--no-sign", tag_name, commit_to_tag], git_dir, env=tag_env)
                print(f"  Created tag '{tag_name}' -> {commit_to_tag[:10]}")
                
        # Update or create branch
        if branch:
            if dry_run:
                print(f"\n  [DRY RUN] Would set branch '{branch}' -> {current_branch_tip[:10]}")
            else:
                run_cmd(["git", "update-ref", f"refs/heads/{branch}", current_branch_tip], git_dir)
                print(f"\n  Updated branch '{branch}' -> {current_branch_tip[:10]}")
                
        print()

if __name__ == "__main__":
    repo_default = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
    default_svn = find_default_svn_dir(repo_default)
    temp_index_default = os.path.join(tempfile.gettempdir(), "vega_migration_git_index")
    
    parser = argparse.ArgumentParser(description="Vega Strike Git Legacy Tag & Branch Migration Tool")
    parser.add_argument("--git-dir", default=repo_default, help=f"Path to Git repository (default: {repo_default})")
    parser.add_argument(
        "--svn-dir",
        "--svn-tags-dir",
        dest="svn_dir",
        default=default_svn,
        help="Path to Subversion repository root (containing 'tags/') or to 'tags/' directory directly. "
             "Can also be set via VEGA_SVN_DIR environment variable."
    )
    parser.add_argument("--temp-index", default=temp_index_default, help=f"Temporary index file path (default: {temp_index_default})")
    parser.add_argument("--execute", action="store_true", help="Perform real changes (default is dry-run)")
    args = parser.parse_args()
    
    execute(
        git_dir=args.git_dir,
        svn_source_dir=args.svn_dir,
        temp_index=args.temp_index,
        dry_run=not args.execute
    )
