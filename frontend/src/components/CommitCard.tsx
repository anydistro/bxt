/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
import {
    faCodeBranch,
    faCubes,
    faMicrochip,
    faTrashCan
} from "@fortawesome/free-solid-svg-icons";
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import { Card, CardProps } from "react-daisyui";

export type ICommitCardProps = CardProps & {
    commit: ICommit;
    onActivate?: (commit: ICommit) => void;
    onDeleteRequested?: (id: string) => void;
};

export default (props: ICommitCardProps) => {
    const section = props.commit.section;

    return (
        <Card
            onClick={() => props.onActivate?.(props.commit)}
            {...props}
            className="h-30 bg-white"
        >
            <div className="w-full flex justify-end">
                <FontAwesomeIcon
                    onClick={(e) => {
                        e.stopPropagation();
                        if (props.onDeleteRequested)
                            return props.onDeleteRequested(props.commit.id);
                    }}
                    className="px-1"
                    icon={faTrashCan}
                />
            </div>
            <Card.Body className="font-bold">
                {props.commit.packages.length} packages
            </Card.Body>
            <span className="space-x-4">
                <FontAwesomeIcon className="px-1" icon={faCodeBranch} />
                {section.branch}
                <FontAwesomeIcon className="px-1" icon={faCubes} />
                {section.repository}
                <FontAwesomeIcon className="px-1" icon={faMicrochip} />
                {section.architecture}
            </span>
        </Card>
    );
};
