# Ecosystem Review

This author tries to keep the entire fork network merged into their upstream, in hopes of providing a unified
codebase for the community to work on.

This document serves as documentation (and note-to-self) on how to do this.


## Steps to perform a review

1. Keep an eye on the ["forks / network members"](https://github.com/afarago/esphome_component_bthome/network/members) 
   page on GitHub, to discover new forks (or forks-of-forks-of....)
2. Ensure all forks are added as remotes to local clone.  
   ```shell
   for USERNAME in \
     juleskers DanCech Kimotu MrSuicideParrot afarago digaus edestecd evlo hostcc hqsone iChenlife jerome83136 konistehrad mathgaming rrooggiieerr t0urista unsplorer \
     ; do 
       git remote add $USERNAME git@github.com:$USERNAME/esphome_component_bthome.git; 
     done;
   ```
3. `git fetch --all --prune`
4. Update local branch to latest public upstream.  
   `git checkout main`  
   `git merge --ff-only juleskers:main`
6. Look for dangling (not yet merged into main) branches anywhere in the known remotes:  
   `git branch --remote --all --no-merged`  
   (shows any branch, including remotes, not yet merged into current, sometimes git is just really powerful...)
7. If anything new has shown up, start a review-branch, from current main  
   `git switch -c ecosystem-review juleskers:main`
8. Merge new stuff  
   ....
9. When finished, merge back, with an explicit merge commit to document the date of the review.  
   ```shell
   git checkout main
   git merge --no-ff ecosystem-review
   git push juleskers
   ```
