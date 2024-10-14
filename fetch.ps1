param(
    $id = "cs2yzG"
)
# pwsh ./fetch.ps1 7tyyDy && xmake b -r shader && xmake b shadertoy && xmake r shadertoy

$resp = Invoke-WebRequest -UseBasicParsing -Uri "https://www.shadertoy.com/shadertoy" `
-Method POST `
-UserAgent "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_10) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/108.0.5163.147 Safari/537.36" `
-Headers @{
  "Referer" = "https://www.shadertoy.com/view/$($id)"
} `
-ContentType "application/x-www-form-urlencoded" `
-Body "s=%7B%20%22shaders%22%20%3A%20%5B%22$($id)%22%5D%20%7D&nt=1&nl=1&np=1"

$conf = $resp.Content | ConvertFrom-Json

foreach ($renderpass in $conf[0].renderpass) {
    $code = $renderpass.code -replace "texture\(", "_texture("
    $out = "src/resource/$($renderpass.name.ToLower()).glsl.in"
    echo "write -> $out"
    echo $code | Out-File -Encoding utf8 -FilePath $out
    foreach ($input in $renderpass.inputs) {
        $ext = Split-Path -Path $input.filepath -Extension
        $out = "src/resource/channel$($input.channel)$($ext)"
        echo "write -> $out"
        Invoke-WebRequest -Uri "https://www.shadertoy.com$($input.filepath)" -OutFile $out
    }
}
