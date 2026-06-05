{
  description = "A S-Expression Parser for C++";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs?ref=nixos-24.05";

    tinycmmc.url = "github:grumbel/tinycmmc";
    tinycmmc.inputs.nixpkgs.follows = "nixpkgs";
    tinycmmc.inputs.flake-utils.follows = "flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils, tinycmmc }:
    tinycmmc.lib.eachSystemWithPkgs (pkgs:
      {
        packages = rec {
          default = sexp-cpp;

          sexp-cpp = pkgs.callPackage ./sexpcpp.nix {
            tinycmmc = tinycmmc.packages.${pkgs.system}.default;
          };
        };
      }
    );
}
