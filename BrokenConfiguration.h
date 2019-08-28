#pragma once

#include "ProperColor.h"

namespace Configuration {
	namespace Visuals {
		namespace Player {
			extern bool Box;
			extern bool FilledBox;
			extern Color CBox;
			extern Color CFilledBox;
			extern bool Name;
			extern bool Health;
			extern bool Weapon;
			extern bool Armor;
			extern bool Ammo;

			namespace Chams {
				extern bool Enabled;

				namespace Enemy {
					extern bool Enabled;
					extern bool XQZ;
				}
				namespace Team {
					extern bool Enabled;
					extern bool XQZ;
				}

			}
		}

		namespace View {
			extern int Fov;
			extern int viewmodelFov;
			extern bool crosshair;
		}
	}
	namespace Misc {
		namespace Spam {
			extern bool locationSpam;
			extern bool radioSpam;
		}
	}

	namespace Menu {
		extern Color Theme;
	}
}