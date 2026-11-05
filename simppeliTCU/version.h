#ifndef VERSION_H
#define VERSION_H

#define FW_STR_HELPER(x) #x
#define FW_STR(x) FW_STR_HELPER(x)

// Release version number. Bump this together with CHANGELOG.md on main to
// automatically create a new git tag and GitHub Release via GitHub Actions.
#ifndef FW_VERSION
#define FW_VERSION "v0.2.0"
#endif

// Optional unquoted commit hash passed via compiler flag, e.g.:
//   -DGIT_HASH_RAW=b3ce9ca
#ifdef GIT_HASH_RAW
#define GIT_HASH FW_STR(GIT_HASH_RAW)
#endif

// Fallback when built locally without injecting the git commit hash (e.g. Arduino IDE)
#ifndef GIT_HASH
#define GIT_HASH "local"
#endif

#define FW_FULL_VERSION FW_VERSION " (" GIT_HASH ")"

#endif // VERSION_H
