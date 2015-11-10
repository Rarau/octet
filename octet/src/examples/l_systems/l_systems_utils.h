
namespace octet
{
	class l_system_utils
	{
	public:

		static string iterate(string axiom, dynarray<string>& rules) {
			dynarray<char> result;

			// Go through each character of the axiom
			for (int i = 0; i < axiom.size(); i++) {
				char current = axiom[i];

				// Find the rule to apply to the current character
				int j = 0;
				for (; j < rules.size(); j++) {
					if (current == rules[j][0]) {
						// We push the applied rule to the result string
						for (int x = 1; x < rules[j].size(); x++) {
							result.push_back(rules[j][x]);
						}
						// We found the rule so we stop searching
						break;
					}
				}

				// If no rule was found we just push the current character
				if (j == rules.size()) {
					result.push_back(current);
				}
			}

			// Add a null terminating character to the end of our result string
			result.push_back(0x00);
			return string(result.data());
		}
	};
}