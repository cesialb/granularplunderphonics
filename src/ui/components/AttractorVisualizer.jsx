import React, { useState, useEffect } from 'react';
import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, Legend, ResponsiveContainer } from 'recharts';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';

// Default mock attractor for testing/placeholder
const defaultAttractor = {
    getDimension: () => 3,
    getState: () => [Math.random() - 0.5, Math.random() - 0.5, Math.random() - 0.5],
    process: () => {},
    analyzePattern: () => ({
        periodicity: Math.random(),
        divergence: Math.random(),
        complexity: Math.random()
    }),
    setUpdateRate: () => {}
};

const AttractorVisualizer = ({ attractor = defaultAttractor, timeWindow = 1000 }) => {
    const [trajectoryData, setTrajectoryData] = useState([]);
    const [dimensions, setDimensions] = useState(3);
    const [patternData, setPatternData] = useState({
        periodicity: 0,
        divergence: 0,
        complexity: 0
    });

    // Colors for different dimensions
    const dimensionColors = ['#2563eb', '#dc2626', '#16a34a'];

    useEffect(() => {
        // Update dimensions when attractor changes
        if (attractor) {
            setDimensions(attractor.getDimension());
            setPatternData(attractor.analyzePattern());
        }
    }, [attractor]);

    useEffect(() => {
        if (!attractor) return;

        // Initialize with some data points
        const initialPoints = Array.from({ length: timeWindow }, (_, i) => {
            const state = attractor.getState();
            attractor.process(); // Advance the system
            return {
                time: i,
                ...state.reduce((acc, val, idx) => ({
                    ...acc,
                    [`dim${idx}`]: val
                }), {})
            };
        });
        setTrajectoryData(initialPoints);

        // Set up periodic updates
        const intervalId = setInterval(() => {
            setTrajectoryData(prevData => {
                const state = attractor.getState();
                attractor.process();

                const newPoint = {
                    time: prevData[prevData.length - 1].time + 1,
                    ...state.reduce((acc, val, idx) => ({
                        ...acc,
                        [`dim${idx}`]: val
                    }), {})
                };

                return [...prevData.slice(1), newPoint];
            });

            setPatternData(attractor.analyzePattern());
        }, 50); // Update every 50ms

        return () => clearInterval(intervalId);
    }, [attractor, timeWindow]);

    if (!attractor) {
        return (
            <Card>
                <CardContent className="p-4">
                    <div className="text-center text-gray-500">
                        No attractor provided. Please initialize an attractor first.
                    </div>
                </CardContent>
            </Card>
        );
    }

    return (
        <div className="space-y-4">
            <Card>
                <CardHeader>
                    <CardTitle>Attractor Trajectory</CardTitle>
                </CardHeader>
                <CardContent>
                    <div className="h-96">
                        <ResponsiveContainer width="100%" height="100%">
                            <LineChart
                                data={trajectoryData}
                                margin={{ top: 5, right: 30, left: 20, bottom: 5 }}
                            >
                                <CartesianGrid strokeDasharray="3 3" />
                                <XAxis
                                    dataKey="time"
                                    label={{ value: 'Time', position: 'bottom' }}
                                />
                                <YAxis
                                    label={{ value: 'State', angle: -90, position: 'left' }}
                                />
                                <Tooltip />
                                <Legend />
                                {Array.from({ length: dimensions }, (_, i) => (
                                    <Line
                                        key={`dim${i}`}
                                        type="monotone"
                                        dataKey={`dim${i}`}
                                        stroke={dimensionColors[i]}
                                        dot={false}
                                        name={`Dimension ${i + 1}`}
                                    />
                                ))}
                            </LineChart>
                        </ResponsiveContainer>
                    </div>
                </CardContent>
            </Card>

            {dimensions >= 2 && (
                <Card>
                    <CardHeader>
                        <CardTitle>Phase Space Plot</CardTitle>
                    </CardHeader>
                    <CardContent>
                        <div className="h-96">
                            <ResponsiveContainer width="100%" height="100%">
                                <LineChart
                                    data={trajectoryData}
                                    margin={{ top: 5, right: 30, left: 20, bottom: 5 }}
                                >
                                    <CartesianGrid strokeDasharray="3 3" />
                                    <XAxis
                                        dataKey="dim0"
                                        label={{ value: 'X', position: 'bottom' }}
                                    />
                                    <YAxis
                                        dataKey="dim1"
                                        label={{ value: 'Y', angle: -90, position: 'left' }}
                                    />
                                    <Tooltip />
                                    <Line
                                        type="monotone"
                                        data={trajectoryData}
                                        stroke="#2563eb"
                                        dot={false}
                                    />
                                </LineChart>
                            </ResponsiveContainer>
                        </div>
                    </CardContent>
                </Card>
            )}

            <Card>
                <CardHeader>
                    <CardTitle>System Analysis</CardTitle>
                </CardHeader>
                <CardContent>
                    <div className="grid grid-cols-3 gap-4">
                        <div className="p-4 rounded-lg bg-gray-100">
                            <div className="text-sm text-gray-500">Periodicity</div>
                            <div className="text-2xl font-bold">
                                {(patternData.periodicity * 100).toFixed(1)}%
                            </div>
                        </div>
                        <div className="p-4 rounded-lg bg-gray-100">
                            <div className="text-sm text-gray-500">Divergence</div>
                            <div className="text-2xl font-bold">
                                {patternData.divergence.toFixed(3)}
                            </div>
                        </div>
                        <div className="p-4 rounded-lg bg-gray-100">
                            <div className="text-sm text-gray-500">Complexity</div>
                            <div className="text-2xl font-bold">
                                {patternData.complexity.toFixed(2)}
                            </div>
                        </div>
                    </div>
                </CardContent>
            </Card>

            <Card>
                <CardHeader>
                    <CardTitle>Controls</CardTitle>
                </CardHeader>
                <CardContent>
                    <div className="space-y-4">
                        <div>
                            <label className="block text-sm font-medium text-gray-700">
                                Update Speed
                            </label>
                            <input
                                type="range"
                                min="1"
                                max="100"
                                defaultValue="50"
                                className="w-full"
                                onChange={(e) => {
                                    attractor.setUpdateRate(parseInt(e.target.value));
                                }}
                            />
                        </div>
                        <div>
                            <label className="block text-sm font-medium text-gray-700">
                                Time Window
                            </label>
                            <select
                                className="mt-1 block w-full rounded-md border-gray-300 shadow-sm"
                                value={timeWindow}
                                onChange={(e) => setTimeWindow(parseInt(e.target.value))}
                            >
                                <option value="500">500ms</option>
                                <option value="1000">1000ms</option>
                                <option value="2000">2000ms</option>
                                <option value="5000">5000ms</option>
                            </select>
                        </div>
                    </div>
                </CardContent>
            </Card>
        </div>
    );
};

export default AttractorVisualizer;