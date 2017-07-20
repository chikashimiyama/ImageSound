#version 120

uniform sampler2DRect tex0;

varying vec2 texCoordVarying;


// mono
void main (void){
    vec2 nextCoord = texCoordVarying;
    nextCoord.x += 1;
    
    vec4 color = texture2DRect(tex0, texCoordVarying);
    vec4 nextColor = texture2DRect(tex0, nextCoord);
    float intensity = (color.r + color.g + color.b) / 3.0;
    float nextIntensity = (nextColor.r +nextColor.g + nextColor.b) / 3.0;
    
    
    float finalIntensity;
    if(abs(nextIntensity - intensity) > 0.03){
        finalIntensity = 0.0;
    }else{
        finalIntensity = 1.0;
    }

    gl_FragColor = vec4(finalIntensity,finalIntensity,finalIntensity,1.0);
}
