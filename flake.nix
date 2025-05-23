{
  description = "Melon OS";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/8f3cf34b8d2e2caf4ae5ee1d1fddc1baab4c5964";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs {
          inherit system;
          overlays = [
            (final: prev: {
              i686-elf-gcc = prev.pkgsCross.i686-embedded.buildPackages.gcc;
            })
          ];
        };
      in {
        packages.default = pkgs.stdenv.mkDerivation {
          pname = "melonOS";
          version = "0.1.0";
          src = ./.;
          nativeBuildInputs = [ pkgs.makeWrapper ];
          buildInputs = [ pkgs.i686-elf-gcc pkgs.qemu ];
          buildPhase = ''
            make
          '';
          installPhase = ''
            mkdir -p $out
            cp -r melonos.img $out/
            cp -r melonfs.img $out/
          '';
        };

        devShells.default = pkgs.mkShell {
          buildInputs = [ pkgs.i686-elf-gcc pkgs.qemu ];
        };
      });
}

