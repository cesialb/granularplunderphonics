import React, { useState, useEffect, useRef } from 'react';
import { Card, CardHeader, CardTitle, CardContent } from '@/components/ui/card';
import { Button } from '@/components/ui/button';
import { Slider } from '@/components/ui/slider';
import { Badge } from '@/components/ui/badge';
import { Select, SelectContent, SelectItem, SelectTrigger, SelectValue } from '@/components/ui/select';
import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer } from 'recharts';

// Lorenz attractor simulation
const simulateLorenz = (state, params, dt) => {
  const [x, y, z] = state;
  const { sigma, rho, beta } = params;

  const dx = sigma * (y - x) * dt;
  const dy = (x * (rho - z) - y) * dt;
  const dz = (x * y - beta * z) * dt;

  return [x + dx, y + dy, z + dz];
};

// Granular parameter simulation
const simulateGrainParameters = (lorenzStates, modulationMatrix) => {
  const { routes } = modulationMatrix;

  // Initial parameter values
  const parameters = {
    grainSize: 50,  // Default 50ms
    grainDensity: 10, // Default 10 grains/sec
    stereoSpread: 0.5, // Default 0.5
    positionOffset: 0.0 // Default 0.0
  };

  // Apply modulation routes to parameters
  return lorenzStates.map(state => {
    const [x, y, z] = state;

    // Reset parameters for this time step
    const timeStepParams = { ...parameters };

    // Apply all modulation routes
    routes.forEach(route => {
      let sourceValue = 0;

      // Determine source value based on route
      if (route.sourceId === 'lorenz_X') sourceValue = x;
      else if (route.sourceId === 'lorenz_Y') sourceValue = y;
      else if (route.sourceId === 'lorenz_Z') sourceValue = z;

      // Apply modulation mode
      let processedValue = sourceValue;
      if (route.mode === 'Unipolar') {
        processedValue = sourceValue * 0.5 + 0.5;
      } else if (route.mode === 'AbsBipolar') {
        processedValue = Math.abs(sourceValue);
      }

      // Apply depth and offset
      processedValue = processedValue * route.depth + route.offset;

      // Apply to destination
      const destParam = route.destinationId.replace('param_', '');
      if (destParam === '2000' || route.destinationId === 'grainSize') {
        // Scale grain size (1-100ms)
        timeStepParams.grainSize = 1 + processedValue * 99;
      } else if (destParam === '2002' || route.destinationId === 'grainDensity') {
        // Scale grain density (0.1-100 Hz)
        timeStepParams.grainDensity = 0.1 + processedValue * 99.9;
      } else if (route.destinationId === 'cloud_spread') {
        timeStepParams.stereoSpread = Math.max(0, Math.min(1, processedValue));
      } else if (route.destinationId === 'cloud_position') {
        timeStepParams.positionOffset = Math.max(0, Math.min(1, processedValue));
      }
    });

    return {
      time: state.time,
      x, y, z,
      ...timeStepParams
    };
  });
};

// Grain visualization component
const GrainVisualizer = ({ grainSize, density, stereoSpread, positionOffset }) => {
  const canvasRef = useRef(null);

  useEffect(() => {
    const canvas = canvasRef.current;
    if (!canvas) return;

    const ctx = canvas.getContext('2d');
    const width = canvas.width;
    const height = canvas.height;

    // Clear canvas
    ctx.clearRect(0, 0, width, height);

    // Calculate number of grains to draw based on density
    const grainCount = Math.min(50, Math.max(5, Math.floor(density / 2)));

    // Convert grain size to visual size (1-100ms → 3-30px)
    const visualGrainSize = 3 + (grainSize / 100) * 27;

    // Draw grains
    for (let i = 0; i < grainCount; i++) {
      // Random position with bias toward positionOffset
      const xPos = Math.random() * 0.3 + positionOffset * 0.7;

      // Y position based on stereo spread
      const yCenter = 0.5;
      const yVariance = stereoSpread * 0.5;
      const yPos = yCenter + (Math.random() * 2 - 1) * yVariance;

      // Draw grain as circle
      ctx.beginPath();
      ctx.arc(
        xPos * width,
        yPos * height,
        visualGrainSize,
        0,
        2 * Math.PI
      );

      // Color based on position (hue)
      const hue = xPos * 360;
      ctx.fillStyle = `hsla(${hue}, 80%, 60%, 0.7)`;
      ctx.fill();
    }
  }, [grainSize, density, stereoSpread, positionOffset]);

  return (
    <canvas
      ref={canvasRef}
      width={400}
      height={200}
      className="w-full h-40 border rounded-lg"
    />
  );
};

// Modulation route editor component
const ModulationRouteEditor = ({ route, index, onUpdate }) => {
  return (
    <div className="border rounded-lg p-3 mb-3">
      <div className="flex items-center justify-between mb-2">
        <div className="flex items-center gap-2">
          <Badge variant="outline">{route.sourceId}</Badge>
          <span className="text-sm">→</span>
          <Badge variant="outline">{route.destinationId}</Badge>
        </div>
        <Badge variant={
          route.mode === 'Bipolar' ? 'default' :
          route.mode === 'Unipolar' ? 'secondary' :
          'outline'
        }>
          {route.mode}
        </Badge>
      </div>

      <div className="space-y-3">
        <div>
          <div className="flex justify-between">
            <span className="text-xs font-medium">Depth</span>
            <span className="text-xs">{Math.round(route.depth * 100)}%</span>
          </div>
          <Slider
            value={[route.depth * 100]}
            min={0}
            max={100}
            step={1}
            onValueChange={(val) => onUpdate(index, 'depth', val[0] / 100)}
          />
        </div>

        <div>
          <div className="flex justify-between">
            <span className="text-xs font-medium">Offset</span>
            <span className="text-xs">{route.offset.toFixed(2)}</span>
          </div>
          <Slider
            value={[(route.offset + 1) * 50]}
            min={0}
            max={100}
            step={1}
            onValueChange={(val) => onUpdate(index, 'offset', (val[0] / 50) - 1)}
          />
        </div>

        <div>
          <span className="text-xs font-medium block mb-1">Mode</span>
          <Select
            value={route.mode}
            onValueChange={(val) => onUpdate(index, 'mode', val)}
          >
            <SelectTrigger className="w-full">
              <SelectValue />
            </SelectTrigger>
            <SelectContent>
              <SelectItem value="Bipolar">Bipolar (-1 to 1)</SelectItem>
              <SelectItem value="Unipolar">Unipolar (0 to 1)</SelectItem>
              <SelectItem value="AbsBipolar">Absolute Bipolar</SelectItem>
            </SelectContent>
          </Select>
        </div>
      </div>
    </div>
  );
};

// Main ModulationMatrix Demo Component
const ModulationMatrixDemo = () => {
  // Lorenz parameters
  const [lorenzParams, setLorenzParams] = useState({
    sigma: 10,
    rho: 28,
    beta: 8/3
  });

  // Modulation matrix configuration
  const [modulationMatrix, setModulationMatrix] = useState({
    routes: [
      {
        sourceId: 'lorenz_X',
        destinationId: 'param_2000', // Grain Size
        depth: 0.5,
        offset: 0.5,
        mode: 'Unipolar'
      },
      {
        sourceId: 'lorenz_Y',
        destinationId: 'param_2002', // Grain Density
        depth: 0.3,
        offset: 0.2,
        mode: 'AbsBipolar'
      },
      {
        sourceId: 'lorenz_Z',
        destinationId: 'cloud_position',
        depth: 0.8,
        offset: 0.5,
        mode: 'Bipolar'
      }
    ]
  });

  // Available presets
  const presets = [
    { name: 'Default', matrix: modulationMatrix },
    {
      name: 'Chaotic Size',
      matrix: {
        routes: [
          { sourceId: 'lorenz_X', destinationId: 'param_2000', depth: 1.0, offset: 0.0, mode: 'Bipolar' },
          { sourceId: 'lorenz_Y', destinationId: 'param_2000', depth: 0.5, offset: 0.5, mode: 'Bipolar' }
        ]
      }
    },
    {
      name: 'Full Chaos',
      matrix: {
        routes: [
          { sourceId: 'lorenz_X', destinationId: 'param_2000', depth: 0.8, offset: 0.2, mode: 'Bipolar' },
          { sourceId: 'lorenz_Y', destinationId: 'param_2002', depth: 0.8, offset: 0.2, mode: 'Unipolar' },
          { sourceId: 'lorenz_Z', destinationId: 'cloud_position', depth: 0.9, offset: 0.5, mode: 'Bipolar' },
          { sourceId: 'lorenz_X', destinationId: 'cloud_spread', depth: 0.7, offset: 0.3, mode: 'AbsBipolar' }
        ]
      }
    },
    {
      name: 'Subtle Motion',
      matrix: {
        routes: [
          { sourceId: 'lorenz_X', destinationId: 'param_2000', depth: 0.2, offset: 0.6, mode: 'Unipolar' },
          { sourceId: 'lorenz_Y', destinationId: 'cloud_position', depth: 0.2, offset: 0.3, mode: 'Unipolar' },
          { sourceId: 'lorenz_Z', destinationId: 'cloud_spread', depth: 0.1, offset: 0.4, mode: 'Unipolar' }
        ]
      }
    }
  ];

  // Simulation state
  const [isRunning, setIsRunning] = useState(false);
  const [simulationData, setSimulationData] = useState([]);
  const [currentParams, setCurrentParams] = useState({
    grainSize: 50,
    grainDensity: 10,
    stereoSpread: 0.5,
    positionOffset: 0.0
  });

  // Run simulation
  useEffect(() => {
    if (!isRunning) return;

    let state = [0.1, 0.1, 0.1]; // Initial Lorenz state
    const dt = 0.005; // Time step
    const historyLength = 100; // Keep last 100 points

    // Initialize history with current state
    let history = Array(historyLength).fill().map((_, i) => ({
      time: i,
      x: state[0],
      y: state[1],
      z: state[2]
    }));

    const simulationInterval = setInterval(() => {
      // Update Lorenz state
      state = simulateLorenz(state, lorenzParams, dt);

      // Add to history and remove oldest point
      history.push({
        time: history[history.length - 1].time + 1,
        x: state[0] / 20, // Scale down for visualization
        y: state[1] / 20,
        z: state[2] / 20
      });

      history = history.slice(-historyLength);

      // Apply modulation to parameters
      const modulatedData = simulateGrainParameters(history, modulationMatrix);
      setSimulationData(modulatedData);

      // Update current parameters display from most recent point
      if (modulatedData.length > 0) {
        const latest = modulatedData[modulatedData.length - 1];
        setCurrentParams({
          grainSize: latest.grainSize,
          grainDensity: latest.grainDensity,
          stereoSpread: latest.stereoSpread,
          positionOffset: latest.positionOffset
        });
      }
    }, 50);

    return () => clearInterval(simulationInterval);
  }, [isRunning, lorenzParams, modulationMatrix]);

  // Update a modulation route
  const updateRoute = (index, property, value) => {
    const newRoutes = [...modulationMatrix.routes];
    newRoutes[index] = {
      ...newRoutes[index],
      [property]: value
    };

    setModulationMatrix({
      ...modulationMatrix,
      routes: newRoutes
    });
  };

  // Load a preset
  const loadPreset = (presetName) => {
    const preset = presets.find(p => p.name === presetName);
    if (preset) {
      setModulationMatrix(preset.matrix);
    }
  };

  return (
    <div className="space-y-6">
      <Card>
        <CardHeader>
          <CardTitle>Modulation Matrix with Lorenz Attractor</CardTitle>
        </CardHeader>
        <CardContent>
          <div className="flex items-center justify-between mb-4">
            <Button
              onClick={() => setIsRunning(!isRunning)}
              variant={isRunning ? "destructive" : "default"}
            >
              {isRunning ? "Stop Simulation" : "Start Simulation"}
            </Button>

            <Select
              value={presets.find(p =>
                JSON.stringify(p.matrix) === JSON.stringify(modulationMatrix)
              )?.name || "Custom"}
              onValueChange={loadPreset}
            >
              <SelectTrigger className="w-[180px]">
                <SelectValue placeholder="Load Preset" />
              </SelectTrigger>
              <SelectContent>
                {presets.map(preset => (
                  <SelectItem key={preset.name} value={preset.name}>
                    {preset.name}
                  </SelectItem>
                ))}
              </SelectContent>
            </Select>
          </div>

          <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
            {/* Lorenz Attractor Visualization */}
            <Card>
              <CardHeader className="pb-2">
                <CardTitle className="text-base">Lorenz Attractor</CardTitle>
              </CardHeader>
              <CardContent>
                <div className="h-64">
                  <ResponsiveContainer width="100%" height="100%">
                    <LineChart data={simulationData}>
                      <CartesianGrid strokeDasharray="3 3" />
                      <XAxis dataKey="time" />
                      <YAxis />
                      <Tooltip />
                      <Line type="monotone" dataKey="x" stroke="#8884d8" dot={false} name="X" />
                      <Line type="monotone" dataKey="y" stroke="#82ca9d" dot={false} name="Y" />
                      <Line type="monotone" dataKey="z" stroke="#ff7300" dot={false} name="Z" />
                    </LineChart>
                  </ResponsiveContainer>
                </div>

                <div className="grid grid-cols-3 gap-2 mt-3">
                  <div>
                    <div className="text-xs font-medium">Sigma</div>
                    <Slider
                      value={[lorenzParams.sigma]}
                      min={1}
                      max={20}
                      step={0.1}
                      onValueChange={(val) => setLorenzParams({...lorenzParams, sigma: val[0]})}
                    />
                  </div>
                  <div>
                    <div className="text-xs font-medium">Rho</div>
                    <Slider
                      value={[lorenzParams.rho]}
                      min={10}
                      max={40}
                      step={0.1}
                      onValueChange={(val) => setLorenzParams({...lorenzParams, rho: val[0]})}
                    />
                  </div>
                  <div>
                    <div className="text-xs font-medium">Beta</div>
                    <Slider
                      value={[lorenzParams.beta]}
                      min={1}
                      max={5}
                      step={0.1}
                      onValueChange={(val) => setLorenzParams({...lorenzParams, beta: val[0]})}
                    />
                  </div>
                </div>
              </CardContent>
            </Card>

            {/* Granular Synthesis Visualization */}
            <Card>
              <CardHeader className="pb-2">
                <CardTitle className="text-base">Granular Synthesis</CardTitle>
              </CardHeader>
              <CardContent>
                <GrainVisualizer
                  grainSize={currentParams.grainSize}
                  density={currentParams.grainDensity}
                  stereoSpread={currentParams.stereoSpread}
                  positionOffset={currentParams.positionOffset}
                />

                <div className="grid grid-cols-2 gap-4 mt-4">
                  <div className="space-y-1">
                    <div className="flex justify-between text-xs">
                      <span className="font-medium">Grain Size</span>
                      <span>{currentParams.grainSize.toFixed(1)} ms</span>
                    </div>
                    <div className="h-2 bg-gray-200 rounded-full">
                      <div
                        className="h-2 bg-blue-500 rounded-full"
                        style={{ width: `${(currentParams.grainSize / 100) * 100}%` }}
                      ></div>
                    </div>
                  </div>

                  <div className="space-y-1">
                    <div className="flex justify-between text-xs">
                      <span className="font-medium">Grain Density</span>
                      <span>{currentParams.grainDensity.toFixed(1)} Hz</span>
                    </div>
                    <div className="h-2 bg-gray-200 rounded-full">
                      <div
                        className="h-2 bg-green-500 rounded-full"
                        style={{ width: `${(currentParams.grainDensity / 100) * 100}%` }}
                      ></div>
                    </div>
                  </div>

                  <div className="space-y-1">
                    <div className="flex justify-between text-xs">
                      <span className="font-medium">Stereo Spread</span>
                      <span>{(currentParams.stereoSpread * 100).toFixed(0)}%</span>
                    </div>
                    <div className="h-2 bg-gray-200 rounded-full">
                      <div
                        className="h-2 bg-purple-500 rounded-full"
                        style={{ width: `${currentParams.stereoSpread * 100}%` }}
                      ></div>
                    </div>
                  </div>

                  <div className="space-y-1">
                    <div className="flex justify-between text-xs">
                      <span className="font-medium">Position Offset</span>
                      <span>{(currentParams.positionOffset * 100).toFixed(0)}%</span>
                    </div>
                    <div className="h-2 bg-gray-200 rounded-full">
                      <div
                        className="h-2 bg-orange-500 rounded-full"
                        style={{ width: `${currentParams.positionOffset * 100}%` }}
                      ></div>
                    </div>
                  </div>
                </div>
              </CardContent>
            </Card>
          </div>

          {/* Modulation Routes */}
          <Card className="mt-6">
            <CardHeader className="pb-2">
              <CardTitle className="text-base">Modulation Routes</CardTitle>
            </CardHeader>
            <CardContent>
              <div className="space-y-4">
                {modulationMatrix.routes.map((route, index) => (
                  <ModulationRouteEditor
                    key={`${route.sourceId}-${route.destinationId}`}
                    route={route}
                    index={index}
                    onUpdate={updateRoute}
                  />
                ))}
              </div>
            </CardContent>
          </Card>
        </CardContent>
      </Card>
    </div>
  );
};

export default ModulationMatrixDemo;