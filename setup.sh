#!/usr/bin/env sh -eux

function dl() {
	(cd dl && curl -s -O https://download.visualstudio.microsoft.com/download/pr/$1)
}

mkdir -p dl

# Microsoft.VisualCpp.CRT.Headers
dl 3711d8f0-a82d-4a7e-a28d-aacdd736535a/425eafe779e747c4dc78821729b8073a/microsoft.visualcpp.crt.headers.vsix

# Microsoft.VisualCpp.CRT.x86.Desktop
dl b258b1d9-06d5-46c7-9e9b-c8540e3c7424/a8bfe1bc0fe0008ae730fbafab70a6f3/microsoft.visualcpp.crt.x86.desktop.vsix

# Microsoft.VisualCpp.CRT.x64.Desktop
dl 540afcb9-22c7-4e2a-a139-405d1b7047fb/c71a55b328b47e044fc767dfbd4421aa/microsoft.visualcpp.crt.x64.desktop.vsix

# Microsoft.VisualCpp.CRT.x86.Store
dl 76292936-286c-458b-b202-89db4595d377/0edf12099363d5224e19029af039e448/microsoft.visualcpp.crt.x86.store.vsix

# Microsoft.VisualCpp.CRT.x64.Store
dl 6c1bc354-dadb-4c15-8ac8-44beae63e096/f511b234f2023b58fb21d6df51c45969/microsoft.visualcpp.crt.x64.store.vsix

# Windows SDK Desktop Headers x64-x86_en-us.msi
dl 713a3452-5333-4ddb-93ed-2fd4b72b6e43/b0f5b8b8a2865aa04d4b5276ea91b20b/windows%20sdk%20desktop%20headers%20x64-x86_en-us.msi
dl 713a3452-5333-4ddb-93ed-2fd4b72b6e43/fdf6a8e345423555a1fe9bd11a9c94fd/e5950ce9ba509b3370ebf328742a5f45.cab

# Windows SDK Desktop Headers x86-x86_en-us.msi
dl 713a3452-5333-4ddb-93ed-2fd4b72b6e43/e6b0e1dc2248375fee0a434f6eee20ac/windows%20sdk%20desktop%20headers%20x86-x86_en-us.msi
dl 713a3452-5333-4ddb-93ed-2fd4b72b6e43/7a8665031d2d65def15ef25c3f79b64c/07a57cdb41ba28cced14005f087267be.cab
dl 713a3452-5333-4ddb-93ed-2fd4b72b6e43/0aceed480f1f252f22aad8b26b7f84b9/2e876dd22fa5e6785f137e3422dd50ec.cab
dl 713a3452-5333-4ddb-93ed-2fd4b72b6e43/d6cb42996647e0edfe27f25bc5acc6a3/4fe4c8b88812f5339018c0eef95acdb9.cab

# Windows SDK Desktop Libs x64-x86_en-us.msi
dl 713a3452-5333-4ddb-93ed-2fd4b72b6e43/5635f4c3761dcef3815287076536d4f5/windows%20sdk%20desktop%20libs%20x64-x86_en-us.msi
dl 713a3452-5333-4ddb-93ed-2fd4b72b6e43/251b482198521679568356092547b155/58314d0646d7e1a25e97c902166c3155.cab

# Windows SDK Desktop Libs x86-x86_en-us.msi
dl 713a3452-5333-4ddb-93ed-2fd4b72b6e43/9a3d8a7c04c9226204d79a15da3163c0/windows%20sdk%20desktop%20libs%20x86-x86_en-us.msi
dl 713a3452-5333-4ddb-93ed-2fd4b72b6e43/c7f175ff489c8f706e900d2d4431a30e/53174a8154da07099db041b9caffeaee.cab

# Windows SDK for Windows Store Apps Headers-x86_en-us.msi
dl 713a3452-5333-4ddb-93ed-2fd4b72b6e43/4ce5755b437a3a613a480af7c1103e07/windows%20sdk%20for%20windows%20store%20apps%20headers-x86_en-us.msi
dl 713a3452-5333-4ddb-93ed-2fd4b72b6e43/505cfe5aa67db62b337bfae4c10a28f5/766c0ffd568bbb31bf7fb6793383e24a.cab
dl 713a3452-5333-4ddb-93ed-2fd4b72b6e43/1f965a26aad97fc01c972bcb70af1794/8125ee239710f33ea485965f76fae646.cab
dl 713a3452-5333-4ddb-93ed-2fd4b72b6e43/7091d0f4a53024a366f07915b02c0b55/c0aa6d435b0851bf34365aadabd0c20f.cab
dl 713a3452-5333-4ddb-93ed-2fd4b72b6e43/67cbae40e40e557bc0ea681c34be6bee/c1c7e442409c0adbf81ae43aa0e4351f.cab

# Windows SDK for Windows Store Apps Libs-x86_en-us.msi
dl 713a3452-5333-4ddb-93ed-2fd4b72b6e43/c4679e064e80a74caa532b62e8a785a5/windows%20sdk%20for%20windows%20store%20apps%20libs-x86_en-us.msi
dl 713a3452-5333-4ddb-93ed-2fd4b72b6e43/2adcc2fbc6be5aec0e5bdd75bedd8514/05047a45609f311645eebcac2739fc4c.cab
dl 713a3452-5333-4ddb-93ed-2fd4b72b6e43/fd8baa43c1510a0672b76444fc2dbc63/0b2a4987421d95d0cb37640889aa9e9b.cab
dl 713a3452-5333-4ddb-93ed-2fd4b72b6e43/c944fde8255a3682aada8b1324edd588/13d68b8a7b6678a368e2d13ff4027521.cab
dl 713a3452-5333-4ddb-93ed-2fd4b72b6e43/889658ed20ed93ed0f8c58247dc08329/463ad1b0783ebda908fd6c16a4abfe93.cab
dl 713a3452-5333-4ddb-93ed-2fd4b72b6e43/c5dbc3cbdf902d73a7d005e992a28f5f/5a22e5cde814b041749fb271547f4dd5.cab
dl 713a3452-5333-4ddb-93ed-2fd4b72b6e43/7bec1f007816190fcf13ebe9c6e1b066/ba60f891debd633ae9c26e1372703e3c.cab
dl 713a3452-5333-4ddb-93ed-2fd4b72b6e43/b45561c41c755ff4ee704976b3a84166/e10768bb6e9d0ea730280336b697da66.cab
dl 713a3452-5333-4ddb-93ed-2fd4b72b6e43/03443f8e9f5bfd3f46d0c4ab4f06f352/f9b24c8280986c0683fbceca5326d806.cab

# Universal CRT Headers Libraries and Sources-x86_en-us.msi
dl 713a3452-5333-4ddb-93ed-2fd4b72b6e43/fce78240c7d946d6b9dc00cbc39f9351/universal%20crt%20headers%20libraries%20and%20sources-x86_en-us.msi
dl 713a3452-5333-4ddb-93ed-2fd4b72b6e43/b4323e5f11456465f5d21671c1df8826/16ab2ea2187acffa6435e334796c8c89.cab
dl 713a3452-5333-4ddb-93ed-2fd4b72b6e43/10727279614e04542efe89a43b057cfe/2868a02217691d527e42fe0520627bfa.cab
dl 713a3452-5333-4ddb-93ed-2fd4b72b6e43/35a9ae0c1ff3978e02bd59f54f3fa25e/6ee7bbee8435130a869cf971694fd9e2.cab
dl 713a3452-5333-4ddb-93ed-2fd4b72b6e43/c7b146f60c3f411ca15fe4e69bb2ccd9/78fa3c824c2c48bd4a49ab5969adaaf7.cab
dl 713a3452-5333-4ddb-93ed-2fd4b72b6e43/07922cf9063cd083791013a86a5e68b0/7afc7b670accd8e3cc94cfffd516f5cb.cab
dl 713a3452-5333-4ddb-93ed-2fd4b72b6e43/b8a9959f35755661b94d16faa0f1954c/80dcdb79b8a5960a384abe5a217a7e3a.cab
dl 713a3452-5333-4ddb-93ed-2fd4b72b6e43/98e9033dff1ed07a498c943071c04d4a/96076045170fe5db6d5dcf14b6f6688e.cab
dl 713a3452-5333-4ddb-93ed-2fd4b72b6e43/c737bef1ea6795b21051c2a2b0de124a/a1e2a83aa8a71c48c742eeaff6e71928.cab
dl 713a3452-5333-4ddb-93ed-2fd4b72b6e43/52ae0f114cf2d67da971bf924fbe3cab/b2f03f34ff83ec013b9e45c7cd8e8a73.cab
dl 713a3452-5333-4ddb-93ed-2fd4b72b6e43/edbc2de89ed1af625ff6afae97eb1cc5/beb5360d2daaa3167dea7ad16c28f996.cab
dl 713a3452-5333-4ddb-93ed-2fd4b72b6e43/a4a6dc958e69f3250f118749fdba6160/eca0aa33de85194cd50ed6e0aae0156f.cab
dl 713a3452-5333-4ddb-93ed-2fd4b72b6e43/4cb7387960667500836138a80a51bcf4/f9ff50431335056fb4fbac05b8268204.cab

mkdir -p sdk
ln -s $PWD/sdk sdk/Contents
for f in dl/*.vsix; do
  unzip "$f" 'Contents/*' -d sdk
done
rm sdk/Contents

ln -s $PWD/sdk sdk/SourceDir
for f in dl/*.msi; do
  msiextract -C sdk "$f"
done
rm sdk/SourceDir

ln -s "$PWD/sdk/VC/Tools/MSVC/14.16.27023" msvc
ln -s "$PWD/sdk/Program Files/Windows Kits/10" winsdk


# rm -fr dl/
