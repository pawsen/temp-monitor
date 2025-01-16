# use with
# nix develop -c $SHELL
{
  description = "nix flake for arduino MK";
  inputs = { nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable"; };
  outputs = { self, nixpkgs }:
    let
      supportedSystems =
        [ "x86_64-linux" "aarch64-linux" "x86_64-darwin" "aarch64-darwin" ];
			# Helper to provide system-specific attributes
      forEachSupportedSystem = f:
        nixpkgs.lib.genAttrs supportedSystems
        (system: f { pkgs = import nixpkgs { inherit system; }; });
    in {
      devShells = forEachSupportedSystem ({ pkgs }: {
        default = pkgs.mkShell.override {
          # Override stdenv in order to change compiler:
          # stdenv = pkgs.clangStdenv;
        }

          {

            # The Nix packages provided in the environment
            packages = with pkgs; [
              (python3.withPackages (ps: with ps; [ pyserial ]))
              arduino-core-unwrapped
              arduino-mk
              # use tio instead of screen, tio -b 9600 /dev/ttyUSB0
              tio
              # keep screen for make monitor
              screen
            ];

            # buildInputs = with pkgs; [];

            # Set any environment variables for your dev shell
            env = {
              ARDMK_DIR = pkgs.arduino-mk;
              ARDUINO_DIR = "${pkgs.arduino-core-unwrapped}/share/arduino";
              ARDUINO_LIB_PATH =
                "${pkgs.arduino-core-unwrapped}/share/arduino/libraries";
            };

            # Add any shell logic you want executed any time the environment is activated
            shellHook = ''
						'';
          };
      });
    };
}
