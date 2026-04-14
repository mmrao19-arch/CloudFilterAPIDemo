#pragma once

using namespace System;

namespace CFAPIWrapper {
	public ref class CFApi sealed
	{
	public:
		// Register the sync root directory. Throws on failure.
		static void RegisterSyncRoot(System::String^ syncRootPath, System::String^ identity, System::String^ displayName);

		// Create a placeholder file at the given path with a declared cloud size.
		static void CreatePlaceholderFile(System::String^ placeholderPath, unsigned long long fileSize);

		// Trigger hydration (download) for the specified file.
		static void TriggerHydration(System::String^ filePath);

		// Notify the native engine about a file state change.
		static void NotifyFileStateChange(System::String^ filePath, int state);
	};
}
