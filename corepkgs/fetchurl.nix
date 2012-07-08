with import <nix/config.nix>;

# Argh, this thing is duplicated (more-or-less) in Nixpkgs.  Need to
# find a way to combine them.

{system, url, outputHash ? "", outputHashAlgo ? "", md5 ? "", sha1 ? "", sha256 ? ""}:

assert (outputHash != "" && outputHashAlgo != "")
    || md5 != "" || sha1 != "" || sha256 != "";

derivation {
  name = baseNameOf (toString url);
  builder = ./fetchurl.sh;

  # Compatibility with Nix <= 0.7.
  id = md5;

  # New-style output content requirements.
  outputHashAlgo = if outputHashAlgo != "" then outputHashAlgo else
      if sha256 != "" then "sha256" else if sha1 != "" then "sha1" else "md5";
  outputHash = if outputHash != "" then outputHash else
      if sha256 != "" then sha256 else if sha1 != "" then sha1 else md5;
  
  inherit system url curl;
}