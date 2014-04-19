 
enablePostProcessing("shaders/pfx.vert", "shaders/pfx.frag")

bloomStrength = 0.015

addIntUniform("bloomEnable")
setIntUniform("bloomEnable", 0)
addFloatUniform("bloomStrength")
setFloatUniform("bloomStrength", bloomStrength)

addFloatUniform("gamma")
setFloatUniform("gamma", 1.1)

radialBlurDist = 1.0
addIntUniform("radialBlurEnable")
setIntUniform("radialBlurEnable", 1)

addFloatUniform("sampleDist")
setFloatUniform("sampleDist", radialBlurDist)

addFloatUniform("sampleStrength")
setFloatUniform("sampleStrength", 2.1)

addIntUniform("onlyAO")
setIntUniform("onlyAO", 0)

onlyAO = 0

guiScene = getScene("guiScene")
addCameraLayer(getScene("mainScene"), getObject("Camera1"), 0)

enableSkybox(1)
loadSkyboxTextures("maps/skybox");

function onSceneUpdate()
	if isKeyPressed("1") then
		setIntUniform("bloomEnable", 1)
		setIntUniform("radialBlurEnable", 0)
	elseif isKeyPressed("2") then
		setIntUniform("bloomEnable", 0)
		setIntUniform("radialBlurEnable", 1)
	end
	
	if isKeyPressed("UP") then
		bloomStrength = bloomStrength + 0.0001
		setFloatUniform("bloomStrength", bloomStrength)
	elseif isKeyPressed("DOWN") then
		bloomStrength = bloomStrength - 0.0001
		setFloatUniform("bloomStrength", bloomStrength)
	end
	
	if isKeyPressed("RIGHT") then
		radialBlurDist = radialBlurDist + 0.01
		setFloatUniform("sampleDist", radialBlurDist)
	elseif isKeyPressed("LEFT") then
		radialBlurDist = radialBlurDist - 0.01
		setFloatUniform("sampleDist", radialBlurDist)
	end
	
	if onKeyDown("SPACE") then
		if onlyAO == 1 then onlyAO = 0 else onlyAO = 1 end
		setIntUniform("onlyAO", onlyAO)
	end
end
