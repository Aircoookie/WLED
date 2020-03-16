## WLED changelog

### Development versions after 0.9.1 release


#### Build 2003141

- Fixed color of main segment returned in JSON API during transition not being target color (closes #765)
- Fixed arlsLock() being called after pixels set in E1.31 (closes #772)
- Fixed HTTP API calls not having an effect if no segment selected (now applies to main segment)

#### Build 2003121

- Created changelog.md - make tracking changes to code easier
- Merged pull request #766 by @pille: Fix E1.31 out-of sequence detection

