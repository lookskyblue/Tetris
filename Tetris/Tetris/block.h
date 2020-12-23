int blocks[7][4][2] =
{ 
	{ // Z block
		{0, 4},
		{0, 5},
		{1, 5}, // Axis
		{1, 6}
	},

	{ // reverse Z block
		{0, 6},
		{0, 5},
		{1, 5}, // Axis
		{1, 4}
	},

	{ // w block
		{0, 5},
		{1, 4},
		{1, 5}, // Axis
		{1, 6}
	},

	{ // reverse L block. 
		{1, 6},
		{1, 4},
		{1, 5}, // Axis
		{0, 4}
	},

	{ // L block
		{1, 4},
		{1, 6},
		{1, 5}, // Axis
		{0, 6}
	},

	{ // square block
		{0, 5},
		{0, 6},
		{1, 5},
		{1, 6}
	},

	{ // bar block. 
		{1, 4},
		{1, 5},
		{1, 6},
		{1, 7}
	}
};
