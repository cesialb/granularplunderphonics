import React, { useState, useEffect } from 'react';
import { Card, CardHeader, CardTitle, CardContent } from '@/components/ui/card';
import { Button } from '@/components/ui/button';
import { Select, SelectContent, SelectItem, SelectTrigger, SelectValue } from '@/components/ui/select';
import { Slider } from '@/components/ui/slider';
import { Badge } from '@/components/ui/badge';
import { ArrowRight, RotateCw, Plus, Minus, Save, Trash2 } from 'lucide-react';

// Mock data structure for demonstration
const defaultModulationSources = [
    { id: 'lorenz_X', name: 'Lorenz X', value: 0.3, isBipolar: true },
    { id: 'lorenz_Y', name: 'Lorenz Y', value: -0.5, isBipolar: true },
    { id: 'lorenz_Z', name: 'Lorenz Z', value: 0.8, isBipolar: true },
    { id: 'lorenz_Periodicity', name: 'Lorenz Periodicity', value: 0.2, isBipolar: false },
    { id: 'lorenz_Complexity', name: 'Lorenz Complexity', value: 0.7, isBipolar: false }
];

const defaultModulationDestinations = [
    { id: 'param_2000', name: 'Grain Size', value: 0.5 },
    { id: 'param_2001', name: 'Grain Shape', value: 0.25 },
    { id: 'param_2002', name: 'Grain Density', value: 0.6 },
    { id: 'cloud_position', name: 'Cloud Position', value: 0.4 },
    { id: 'cloud_spread', name: 'Cloud Spread', value: 0.3 }
];

const defaultModulationRoutes = [
    { sourceId: 'lorenz_X', destinationId: 'param_2000', depth: 0.7, offset: 0.0, mode: 'Bipolar' },
    { sourceId: 'lorenz_Y', destinationId: 'param_2002', depth: 0.5, offset: 0.2, mode: 'Unipolar' },
    { sourceId: 'lorenz_Z', destinationId: 'cloud_position', depth: 0.3, offset: -0.1, mode: 'AbsBipolar' }
];

const defaultPresets = [
    { name: 'Default', routes: [...defaultModulationRoutes] },
    { name: 'Chaos Control', routes: [
            { sourceId: 'lorenz_X', destinationId: 'param_2000', depth: 0.9, offset: 0.0, mode: 'Bipolar' },
            { sourceId: 'lorenz_Y', destinationId: 'param_2001', depth: 0.9, offset: 0.0, mode: 'Bipolar' },
            { sourceId: 'lorenz_Z', destinationId: 'param_2002', depth: 0.9, offset: 0.0, mode: 'Bipolar' }
        ]},
    { name: 'Subtle Motion', routes: [
            { sourceId: 'lorenz_X', destinationId: 'param_2000', depth: 0.2, offset: 0.5, mode: 'Unipolar' },
            { sourceId: 'lorenz_Y', destinationId: 'cloud_position', depth: 0.3, offset: 0.0, mode: 'Unipolar' },
            { sourceId: 'lorenz_Complexity', destinationId: 'cloud_spread', depth: 0.4, offset: 0.0, mode: 'Unipolar' }
        ]},
    { name: 'Rhythmic Pulse', routes: [
            { sourceId: 'lorenz_Z', destinationId: 'param_2002', depth: 1.0, offset: 0.0, mode: 'AbsBipolar' },
            { sourceId: 'lorenz_Periodicity', destinationId: 'param_2000', depth: 0.6, offset: 0.3, mode: 'Unipolar' }
        ]}
];

// Visual indicator component for modulation sources
const ModulationIndicator = ({ value, isBipolar }) => {
    const normalizedValue = isBipolar ? (value + 1) / 2 : value;

    return (
        <div className="w-full h-4 bg-gray-200 rounded relative overflow-hidden">
            {isBipolar && (
                <div className="absolute top-0 bottom-0 left-1/2 w-px bg-gray-400" />
            )}
            <div
                className="absolute top-0 bottom-0 bg-blue-500"
                style={{
                    left: isBipolar ? '50%' : '0',
                    width: isBipolar ? `${Math.abs(value) * 50}%` : `${value * 100}%`,
                    transform: isBipolar && value < 0 ? 'translateX(-100%)' : 'none'
                }}
            />
        </div>
    );
};

// Connection visualization component
const ModulationConnection = ({ source, destination, depth, mode, offset }) => {
    let modeColor = 'bg-blue-500';
    if (mode === 'Unipolar') modeColor = 'bg-green-500';
    if (mode === 'AbsBipolar') modeColor = 'bg-purple-500';

    return (
        <div className="flex items-center p-2 rounded-lg border mb-2 bg-gray-50">
            <div className="w-1/4 pr-2">
                <div className="font-medium text-sm truncate">{source}</div>
            </div>

            <div className="flex-grow flex items-center justify-center gap-1">
                <div className={`h-2 rounded-full ${modeColor}`} style={{width: `${depth * 100}%`}}></div>
                <ArrowRight className="h-4 w-4 text-gray-500" />
            </div>

            <div className="w-1/4 pl-2">
                <div className="font-medium text-sm truncate">{destination}</div>
            </div>
        </div>
    );
};

const ModulationMatrixVisualizer = ({ sources = defaultModulationSources,
                                        destinations = defaultModulationDestinations,
                                        initialRoutes = defaultModulationRoutes,
                                        initialPresets = defaultPresets,
                                        onRouteChange = () => {} }) => {
    const [routes, setRoutes] = useState(initialRoutes);
    const [presets, setPresets] = useState(initialPresets);
    const [selectedPreset, setSelectedPreset] = useState('Default');
    const [routeBeingEdited, setRouteBeingEdited] = useState(null);

    // Map of route identifiers
    const routeMap = routes.reduce((map, route) => {
        map[`${route.sourceId}->${route.destinationId}`] = route;
        return map;
    }, {});

    // Function to find source/destination by ID
    const getSourceById = (id) => sources.find(s => s.id === id) || { name: 'Unknown' };
    const getDestinationById = (id) => destinations.find(d => d.id === id) || { name: 'Unknown' };

    // Add a new route
    const addRoute = () => {
        if (sources.length === 0 || destinations.length === 0) return;

        // Find first available source and destination that aren't already connected
        for (const source of sources) {
            for (const destination of destinations) {
                const routeKey = `${source.id}->${destination.id}`;
                if (!routeMap[routeKey]) {
                    const newRoute = {
                        sourceId: source.id,
                        destinationId: destination.id,
                        depth: 0.5,
                        offset: 0.0,
                        mode: 'Bipolar'
                    };

                    const updatedRoutes = [...routes, newRoute];
                    setRoutes(updatedRoutes);
                    onRouteChange(updatedRoutes);
                    return;
                }
            }
        }

        // If we get here, all possible routes already exist
        alert('All possible routes are already created.');
    };

    // Remove a route
    const removeRoute = (sourceId, destinationId) => {
        const updatedRoutes = routes.filter(
            route => !(route.sourceId === sourceId && route.destinationId === destinationId)
        );
        setRoutes(updatedRoutes);
        onRouteChange(updatedRoutes);
    };

    // Update a route's parameters
    const updateRoute = (sourceId, destinationId, paramName, value) => {
        const updatedRoutes = routes.map(route => {
            if (route.sourceId === sourceId && route.destinationId === destinationId) {
                return { ...route, [paramName]: value };
            }
            return route;
        });

        setRoutes(updatedRoutes);
        onRouteChange(updatedRoutes);
    };

    // Load a preset
    const loadPreset = (presetName) => {
        const preset = presets.find(p => p.name === presetName);
        if (preset) {
            setRoutes(preset.routes);
            onRouteChange(preset.routes);
            setSelectedPreset(presetName);
        }
    };

    // Save current routes as a new preset
    const saveAsPreset = () => {
        const presetName = prompt('Enter a name for this preset:');
        if (!presetName) return;

        const newPreset = {
            name: presetName,
            routes: [...routes]
        };

        const updatedPresets = [...presets, newPreset];
        setPresets(updatedPresets);
        setSelectedPreset(presetName);
    };

    return (
        <div className="space-y-4">
            <Card>
                <CardHeader>
                    <div className="flex justify-between items-center">
                        <CardTitle>Modulation Matrix</CardTitle>
                        <div className="flex gap-2">
                            <Select value={selectedPreset} onValueChange={loadPreset}>
                                <SelectTrigger className="w-[180px]">
                                    <SelectValue placeholder="Select preset" />
                                </SelectTrigger>
                                <SelectContent>
                                    {presets.map(preset => (
                                        <SelectItem key={preset.name} value={preset.name}>
                                            {preset.name}
                                        </SelectItem>
                                    ))}
                                </SelectContent>
                            </Select>
                            <Button size="sm" onClick={saveAsPreset}>
                                <Save className="h-4 w-4 mr-1" />
                                Save
                            </Button>
                        </div>
                    </div>
                </CardHeader>
                <CardContent>
                    <div className="flex justify-between mb-4">
                        <Button variant="outline" onClick={addRoute}>
                            <Plus className="h-4 w-4 mr-1" />
                            Add Route
                        </Button>
                    </div>

                    {/* Routes display */}
                    <div className="space-y-4">
                        {routes.length === 0 ? (
                            <div className="text-center py-8 text-gray-500">
                                No modulation routes configured. Click "Add Route" to create one.
                            </div>
                        ) : (
                            routes.map((route, index) => {
                                const source = getSourceById(route.sourceId);
                                const destination = getDestinationById(route.destinationId);

                                return (
                                    <Card key={`${route.sourceId}-${route.destinationId}`} className="border-gray-200">
                                        <CardContent className="p-4">
                                            <div className="flex items-center justify-between mb-2">
                                                <div className="flex items-center gap-2">
                                                    <Badge variant={route.mode === 'Bipolar' ? 'default' :
                                                        route.mode === 'Unipolar' ? 'secondary' : 'outline'}>
                                                        {route.mode}
                                                    </Badge>
                                                    <span className="text-sm text-gray-500">
                            Depth: {Math.round(route.depth * 100)}%
                          </span>
                                                </div>
                                                <Button
                                                    variant="ghost"
                                                    size="sm"
                                                    onClick={() => removeRoute(route.sourceId, route.destinationId)}
                                                >
                                                    <Trash2 className="h-4 w-4 text-red-500" />
                                                </Button>
                                            </div>

                                            <ModulationConnection
                                                source={source.name}
                                                destination={destination.name}
                                                depth={route.depth}
                                                mode={route.mode}
                                                offset={route.offset}
                                            />

                                            <div className="mt-4 space-y-4">
                                                <div>
                                                    <div className="flex justify-between mb-1">
                                                        <label className="text-sm font-medium">Depth</label>
                                                        <span className="text-xs">{Math.round(route.depth * 100)}%</span>
                                                    </div>
                                                    <Slider
                                                        value={[route.depth * 100]}
                                                        min={0}
                                                        max={100}
                                                        step={1}
                                                        onValueChange={(value) => updateRoute(
                                                            route.sourceId,
                                                            route.destinationId,
                                                            'depth',
                                                            value[0] / 100
                                                        )}
                                                    />
                                                </div>

                                                <div>
                                                    <div className="flex justify-between mb-1">
                                                        <label className="text-sm font-medium">Offset</label>
                                                        <span className="text-xs">{route.offset.toFixed(2)}</span>
                                                    </div>
                                                    <Slider
                                                        value={[(route.offset + 1) * 50]}
                                                        min={0}
                                                        max={100}
                                                        step={1}
                                                        onValueChange={(value) => updateRoute(
                                                            route.sourceId,
                                                            route.destinationId,
                                                            'offset',
                                                            (value[0] / 50) - 1
                                                        )}
                                                    />
                                                </div>

                                                <div>
                                                    <label className="text-sm font-medium block mb-1">Mode</label>
                                                    <Select
                                                        value={route.mode}
                                                        onValueChange={(value) => updateRoute(
                                                            route.sourceId,
                                                            route.destinationId,
                                                            'mode',
                                                            value
                                                        )}
                                                    >
                                                        <SelectTrigger>
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
                                        </CardContent>
                                    </Card>
                                );
                            })
                        )}
                    </div>
                </CardContent>
            </Card>

            <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
                {/* Sources display */}
                <Card>
                    <CardHeader>
                        <CardTitle>Modulation Sources</CardTitle>
                    </CardHeader>
                    <CardContent>
                        <div className="space-y-4">
                            {sources.map(source => (
                                <div key={source.id} className="flex items-center space-x-2">
                                    <div className="w-1/3 text-sm font-medium truncate">{source.name}</div>
                                    <div className="flex-grow">
                                        <ModulationIndicator value={source.value} isBipolar={source.isBipolar} />
                                    </div>
                                    <div className="w-12 text-right text-sm">
                                        {source.value.toFixed(2)}
                                    </div>
                                </div>
                            ))}
                        </div>
                    </CardContent>
                </Card>

                {/* Destinations display */}
                <Card>
                    <CardHeader>
                        <CardTitle>Modulation Destinations</CardTitle>
                    </CardHeader>
                    <CardContent>
                        <div className="space-y-4">
                            {destinations.map(dest => (
                                <div key={dest.id} className="flex items-center space-x-2">
                                    <div className="w-1/3 text-sm font-medium truncate">{dest.name}</div>
                                    <div className="flex-grow">
                                        <ModulationIndicator value={dest.value} isBipolar={false} />
                                    </div>
                                    <div className="w-12 text-right text-sm">
                                        {dest.value.toFixed(2)}
                                    </div>
                                </div>
                            ))}
                        </div>
                    </CardContent>
                </Card>
            </div>
        </div>
    );
};

export default ModulationMatrixVisualizer;